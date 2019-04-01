 #include<sys/types.h>
 #include<sys/stat.h>
 #include<sys/file.h>
 #include<dirent.h>
 #include<fcntl.h>
 #include<math.h>
 #include<unistd.h>
 #include <ctype.h>
 #include<string.h>
 #include<stdio.h>
 #include<time.h>
 #include<gdk/gdkkeysyms.h>
 #include<gtk/gtk.h>

 #include<stdlib.h>

void add_to_list(GtkWidget *gtk_list);
//static GtkWidget *timep=NULL; //保存时间
static GdkPixmap *pixmap=NULL;  //CPU使用曲线图
static GdkPixmap *pixmap2=NULL;  //CPU使用曲线图
static GdkPixmap *pixmap3=NULL;  //CPU使用曲线图

GdkGC  *gc1,*gc2,*gc3;
GdkColor color,color2,color3;         //表示一个颜色系
static float cpu_util=0,mem_util=0,swap_util=0;
static long cpu_util_data[120];
static long mem_util_data[120];
static long swap_util_data[120];
enum
{
  name_column = 0,
  pid_column,
  ppid_column,
  state_column,
  mem_column,
  prio_column,
  total_columns
};
struct process{         //存放进程信息
    char pid[5];        //进程识别号
    char ppid[20];      //父进程识别号
    char name[20];      //应用程序名
    char state[20];     //状态
    char mem[20];       //内存占用
    char prio[20];      //进程动态优先级
}list[1000];
void call_reboot(GtkMenuItem *menu,gpointer user_data){
    system("reboot");
}
void call_shutdown(GtkMenuItem *menu,gpointer user_data){
    system("halt");
}
void call_fresh(GtkWidget *button,GtkWidget *tree_view){
    add_to_list(tree_view);
}
void call_delete(GtkWidget *button, GtkTreeSelection *widget){
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkListStore *store;
    char *value;   
    char *comm;
    value=(char *)malloc(20*sizeof(char));
    comm=(char *)malloc(20*sizeof(char));  
    memset(value,0,sizeof(char)*20);    
    memset(comm,0,sizeof(char)*0);    

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {

        gtk_tree_model_get(model, &iter, pid_column, &value,  -1);
        store = GTK_LIST_STORE(model);
        sprintf(comm,"kill %s",value);
        gtk_list_store_remove(store, &iter);
        system(comm);            
        g_free(value);
        g_free(comm);
    }
}
gboolean settime(gpointer data)
{
    time_t times;
    struct tm *p_time;
    time(&times);
    p_time = localtime(&times);

    gchar *text_data = g_strdup_printf(\
        "%04d-%02d-%02d",\
        (1900+p_time->tm_year),(1+p_time->tm_mon),(p_time->tm_mday));
    //strcat(buf,ctime(&timep));
    gchar *text_time = g_strdup_printf(\
    "%02d:%02d:%02d",\
    (p_time->tm_hour), (p_time->tm_min), (p_time->tm_sec));

    gchar *text_markup = g_strdup_printf("%s    %s", text_data,text_time);

    gtk_label_set_markup(GTK_LABEL(data), text_markup);

    return TRUE;
}
int search_name(char *name){
    int i=0;
    for(i=0;name[i]!=0;i++)
    {
        if(isalpha(name[i])||name[i]=='.')
            return 0;
    }
    return 1;
}
int get_proc(void){
    DIR *dir;
    struct dirent *ptr;
    int  file,err;
    int  i=0,total=0,j;
    char statm_path[50];
    char status_path[50];
    char stat_path[50];
    char buf[1000];
    memset(buf,0,sizeof(char)*1000);   
    dir=opendir("/proc");
    while((ptr=readdir(dir))!=NULL&&i<1000){
        if(search_name(ptr->d_name)){
            strcpy(list[i++].pid,ptr->d_name);
            total++;
        }
    }
    printf("%d %d",i,total);
    closedir(dir);
    for(i=0;i<total;i++){
        strcpy(statm_path,"/proc/");    //设定statm文件路径
        strcat(statm_path,list[i].pid);
        strcat(statm_path,"/statm");
        strcpy(status_path,"/proc/");   //设定status文件路径
        strcat(status_path,list[i].pid);
        strcat(status_path,"/status"); 
        strcpy(stat_path,"/proc/");     //设定stat文件路径
        strcat(stat_path,list[i].pid);
        strcat(stat_path,"/stat"); 
        file=open(status_path,O_RDONLY);//读取status文件信息
        err=read(file,buf,1000);
        strtok(buf,":");
        strcpy(list[i].name,strtok(NULL,"\n"));
        strtok(NULL,":");
        strcpy(list[i].state,strtok(NULL,"\n"));
        strtok(NULL,":");
        strtok(NULL,"\n");
        strtok(NULL,":");
        strtok(NULL,"\n");
        strtok(NULL,":");
        strtok(NULL,"\n");
        strtok(NULL,":");
        strtok(NULL,"\n");
        strtok(NULL,":");
        strcpy(list[i].ppid,strtok(NULL,"\n"));
        close(file);
        memset(buf,0,sizeof(char)*1000);
        file=open(statm_path,O_RDONLY);//读取statm文件信息
        err=read(file,buf,1000);
        strcpy(list[i].mem,strtok(buf," "));
        strcat(list[i].mem,"kb");
        close(file);
        memset(buf,0,sizeof(char)*1000);
        file=open(stat_path,O_RDONLY);//读取stat文件信息
        err=read(file,buf,1000);
        strtok(buf," ");  
        for(j=0;j<16;j++)
        {
        strtok(NULL," ");  
        }
        strcpy(list[i].prio,strtok(NULL," "));
        close(file);
    }
    return total;
}
void show_sys(GtkWidget *text){
    int fd_name,fd_ver;
    char *buf,*med;
    buf=(char *)malloc(1000*sizeof(char));
    med=(char *)malloc(1000*sizeof(char));  
    memset(buf,0,sizeof(char)*1000);    
    strcat(buf,"\n  主机名:");
    fd_name=open("/proc/sys/kernel/hostname",O_RDONLY);
    read(fd_name,med,1000);
    close(fd_name);
    strcat(buf,med);
    memset(med,0,sizeof(char)*1000);
    strcat(buf,"\n系统版本号:");
    fd_ver=open("/proc/sys/kernel/osrelease",O_RDONLY);
    read(fd_ver,med,1000);
    close(fd_ver);
    strcat(buf,med);
    gtk_label_set_text(GTK_LABEL(text),g_locale_to_utf8(buf,-1,0,0,0));
    memset(buf,0,sizeof(char)*1000);    
    free(buf);
    memset(med,0,sizeof(char)*1000);
    free(med);
}
void show_cpu(GtkWidget *text){
    int fd_cpu;
    char *buf,*med;
    buf=(char *)malloc(1000*sizeof(char));
    med=(char *)malloc(1000*sizeof(char));   
    memset(buf,0,sizeof(char)*1000);   
    strcat(buf,"\n  ");
    fd_cpu=open("/proc/cpuinfo",O_RDONLY);
    read(fd_cpu,med,1000);
    close(fd_cpu);
    strtok(med,"\n");
    for(int i=0;i<7;i++)
    {
    strcat(buf,strtok(NULL,"\n"));
    strcat(buf,"\n  ");
    }
    gtk_label_set_text(GTK_LABEL(text),g_locale_to_utf8(buf,-1,0,0,0));
    memset(buf,0,sizeof(char)*1000);
    free(buf);
    memset(med,0,sizeof(char)*1000);
    free(med);
}       
gboolean cal_mem_util(gpointer result){
    int file;
    file=open("/proc/meminfo",O_RDONLY);
    char *buf;
    float memtotal,memfree,swaptotal,swapfree;

    buf=(char *)malloc(1000*sizeof(char));
    memset(buf,0,sizeof(char)*1000);   
    read(file,buf,1000);
    strtok(buf,":");
    sscanf(strtok(NULL,"\n"),"%f",&memtotal);
    strtok(NULL,":");
    sscanf(strtok(NULL,"\n"),"%f",&memfree);  
    for(int i=0;i<12;i++)
    {
    strtok(NULL,"\n");
    }
    strtok(NULL,":");
    sscanf(strtok(NULL,"\n"),"%f",&swaptotal);
    strtok(NULL,":");
    sscanf(strtok(NULL,"\n"),"%f",&swapfree); 
    close(file);
    mem_util=100.0*(memtotal-memfree)/memtotal;
    swap_util=100.0*(swaptotal-swapfree)/swaptotal;
    memset(buf,0,sizeof(char)*1000);   
    sprintf(buf,"mem使用率:%0.2f    swap使用率:%0.2f",mem_util,swap_util);
    gtk_label_set_text(GTK_LABEL(result),buf);
    free(buf);
    return TRUE;
}
gboolean cal_cpu_util(gpointer result){
    int file;
    file=open("/proc/stat",O_RDONLY);
    char *buf;
    char name[20];
    long user_t1,user_t2,nice_t1,nice_t2,system_t1,idle_t1,iowait_t1,iowait_t2;
    long system_t2,idle_t2;
    float total_t1,total_t2;

    buf=(char *)malloc(1000*sizeof(char));
    memset(buf,0,sizeof(char)*1000);   
    read(file,buf,1000);
    sscanf(buf,"%s %ld %ld %ld %ld %ld",name,&user_t1,&nice_t1,&system_t1,&idle_t1,&iowait_t1);
    total_t1=user_t1+nice_t1+system_t1+idle_t1+iowait_t1;
    close(file);
    memset(buf,0,sizeof(char)*1000);   
    usleep(50000);//延迟500毫秒
    file=open("/proc/stat",O_RDONLY);
    read(file,buf,1000);
    sscanf(buf,"%s %ld %ld %ld %ld %ld",name,&user_t2,&nice_t2,&system_t2,&idle_t2,&iowait_t2);
    close(file);
    total_t2=user_t2+nice_t2+system_t2+idle_t2+iowait_t2;
    cpu_util=100.0*((total_t2-total_t1)-(float)(idle_t2-idle_t1))/(total_t2-total_t1);
    memset(buf,0,sizeof(char)*1000);   
    sprintf(buf,"CPU使用率:%0.2f",cpu_util);
    gtk_label_set_text(GTK_LABEL(result),buf);
    free(buf);
    return TRUE;
}
gboolean show_sys_time(gpointer text){
    int file;
    char *buf,*med;
    int time_past;
    int hour,min,sec;
    time_t timep;
    buf=(char *)malloc(1000*sizeof(char));
    med=(char *)malloc(1000*sizeof(char));   
    memset(buf,0,sizeof(char)*1000);
    strcat(buf,"系统开启时间:");
    file=open("/proc/uptime",O_RDONLY);   
    read(file,med,100);
    time_past=atoi(strtok(med,"."));
    time(&timep);
    timep-=time_past;
    strcat(buf,ctime(&timep));
    strcat(buf,"系统运行时间:");
    hour=time_past/3600;
    min=(time_past-hour*3600)/60;
    sec=time_past%60;
    sprintf(med,"%d:%d:%d",hour,min,sec);
    strcat(buf,med);
    gtk_label_set_text(GTK_LABEL(text),buf);
    close(file);
    free(buf);
    free(med);
    return TRUE;
}

static gint obtable_text_number_sort(const gchar *text_a,const gchar *text_b)
{
    gint retval = 0;
     
    if(text_a == NULL && text_b == NULL)
        retval = 0;   
    else if(text_a == NULL) retval = -1;
    else if(text_b == NULL) retval = 1;
    else {
        int a;
        int b;
         
        a = atoi(text_a);
        b = atoi(text_b);    
        //retval = strcasecmp(text_a,text_b);
        retval = a-b;
    }
     
    return retval;   
}
 
 
//FIXME:3列数据排序的函数static 
gint obtable_tree_sort(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer user_data)
{
    const gchar *text_a;
    const gchar *text_b;
     
    gtk_tree_model_get(model,a,GTK_TREE_VIEW_COLUMN(user_data)->sort_column_id,&text_a,-1);
    gtk_tree_model_get(model,b,GTK_TREE_VIEW_COLUMN(user_data)->sort_column_id,&text_b,-1);
     
    return     obtable_text_number_sort(text_a,text_b);
}

static void column_sort(GtkTreeViewColumn *column,GtkListStore *store){
    static GtkSortType next_sort;
    if(column->sort_column_id!=name_column){
        gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store),column->sort_column_id,obtable_tree_sort,column,NULL);
        next_sort=GTK_SORT_ASCENDING;
    }
    else
    {
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),column->sort_column_id,next_sort);//按值升序排列
        if(next_sort==GTK_SORT_ASCENDING)next_sort=GTK_SORT_DESCENDING;
        else next_sort=GTK_SORT_ASCENDING;
        //每次g—signal调用gtk_tree_view_column_get_sort_order均为0，迷之bug
        // if(gtk_tree_view_column_get_sort_order (column)==GTK_SORT_DESCENDING){
        //     printf("%d    ",gtk_tree_view_column_get_sort_order (column));
        //     gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),column->sort_column_id,GTK_SORT_ASCENDING);//按值升序排列
        //     printf("%d    ",gtk_tree_view_column_get_sort_order (column));
        // }
        // else{
        //     printf("%d    ",gtk_tree_view_column_get_sort_order (column));
        //     gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),column->sort_column_id,GTK_SORT_DESCENDING);//按值降序排列
        //     gtk_tree_view_column_set_sort_order (column,GTK_SORT_ASCENDING);
        //     printf("%d    ",gtk_tree_view_column_get_sort_order (column));
        // } 
    }
}
gboolean draw_cpu_util(gpointer widget){
    static long flag=0,pos_now=0;
    long pos_draw=0;
    gdk_draw_rectangle(GTK_WIDGET(widget)->window,GTK_WIDGET(widget)->style->white_gc,TRUE,0,0,GTK_WIDGET(widget)->allocation.width,GTK_WIDGET(widget)->allocation.height);

    // if(flag==0){
    //     for (int i = 0; i < 120; i++) {
    //     cpu_util_data[i] = 0;
    //     flag = 1;
    //     }
    // }
    cpu_util_data[pos_now] = (long)cpu_util;
    pos_now++;
    pos_now=pos_now%120;
    pos_draw=pos_now;
    for (int i = 0; i < 119; i++) {
        gdk_draw_line(GTK_WIDGET(widget)->window, gc1,
              5*i, 100-cpu_util_data[pos_draw % 120],
              5*(i + 1), 100-cpu_util_data[(pos_draw + 1) % 120]);
        pos_draw++;
        if (pos_draw == 120)
        pos_draw = 0;
    }
    printf("cpu:%ld %ld\n",pos_now,pos_draw);
    return 1;
}
gboolean draw_mem_util(gpointer widget){
    static long flag=0,pos_now=0;
    long pos_draw=0;
    gdk_draw_rectangle(GTK_WIDGET(widget)->window,GTK_WIDGET(widget)->style->white_gc,TRUE,0,0,GTK_WIDGET(widget)->allocation.width,GTK_WIDGET(widget)->allocation.height);

    if(flag==0){
        for (int i = 0; i < 120; i++) {
        mem_util_data[i] = 0;
        flag = 1;
        }
    }
    mem_util_data[pos_now] = (long)mem_util;
    pos_now++;
    pos_now=pos_now%120;
    pos_draw=pos_now;
    for (int i = 0; i < 119; i++) {
        gdk_draw_line(GTK_WIDGET(widget)->window, gc2,
              5*i, 100-mem_util_data[pos_draw % 120],
              5*(i + 1), 100-mem_util_data[(pos_draw + 1) % 120]);
        pos_draw++;
        if (pos_draw == 120)
        pos_draw = 0;
    }
    printf("mem:%ld %ld\n",pos_now,pos_draw);
    return 1;
}
gboolean draw_swap_util(gpointer widget){
    static long flag=0,pos_now=0;
    long pos_draw=0;
    gdk_draw_rectangle(GTK_WIDGET(widget)->window,GTK_WIDGET(widget)->style->white_gc,TRUE,0,0,GTK_WIDGET(widget)->allocation.width,GTK_WIDGET(widget)->allocation.height);

    if(flag==0){
        for (int i = 0; i < 120; i++) {
        swap_util_data[i] = 0;
        flag = 1;
        }
    }
    swap_util_data[pos_now] = (long)swap_util;
    pos_now++;
    pos_now=pos_now%120;
    pos_draw=0;
    for (int i = 0; i < 119; i++) {
        gdk_draw_line(GTK_WIDGET(widget)->window, gc3,
              5*i, 100-swap_util_data[pos_draw % 120],
              5*(i + 1), 100-swap_util_data[(pos_draw + 1) % 120]);
        pos_draw++;
        if (pos_draw == 120)
        pos_draw = 0;
    }
    return 1;
}

void draw_conf(GtkWidget *widget,GdkEventConfigure *event){
    if(pixmap)g_object_unref(pixmap);
    pixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);
    gdk_draw_rectangle(pixmap,widget->style->white_gc,TRUE,0,0,widget->allocation.width,widget->allocation.height);
    gc1=gdk_gc_new(widget->window);
    color.red=30000;
    color.green=3000;
    color.blue=3000;
    gdk_gc_set_rgb_fg_color(gc1,&color);
}
void draw_expose(GtkWidget *widget,GdkEventExpose *event,gpointer data){
    gdk_draw_drawable(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],pixmap,0,0,0,0,widget->allocation.width,widget->allocation.height);
    g_timeout_add(1000,draw_cpu_util,(gpointer)widget);
}
void draw_conf2(GtkWidget *widget,GdkEventConfigure *event){
    if(pixmap2)g_object_unref(pixmap2);
    pixmap2=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);
    gdk_draw_rectangle(pixmap2,widget->style->white_gc,TRUE,0,0,widget->allocation.width,widget->allocation.height);
    gc2=gdk_gc_new(widget->window);
    color2.red=3000;
    color2.green=30000;
    color2.blue=3000;
    gdk_gc_set_rgb_fg_color(gc2,&color2);
}
void draw_expose2(GtkWidget *widget,GdkEventExpose *event,gpointer data){
    gdk_draw_drawable(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],pixmap2,0,0,0,0,widget->allocation.width,widget->allocation.height);
    g_timeout_add(1000,draw_mem_util,(gpointer)widget);
}
void draw_conf3(GtkWidget *widget,GdkEventConfigure *event){
    if(pixmap3)g_object_unref(pixmap3);
    pixmap3=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);
    gdk_draw_rectangle(pixmap3,widget->style->white_gc,TRUE,0,0,widget->allocation.width,widget->allocation.height);
    gc3=gdk_gc_new(widget->window);
    color3.red=3000;
    color3.green=3000;
    color3.blue=30000;
    gdk_gc_set_rgb_fg_color(gc3,&color3);
}
void draw_expose3(GtkWidget *widget,GdkEventExpose *event,gpointer data){
    gdk_draw_drawable(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)],pixmap3,0,0,0,0,widget->allocation.width,widget->allocation.height);
    g_timeout_add(1000,draw_swap_util,(gpointer)widget);
}
void init_list(GtkWidget *list)
{
    GtkListStore *store;
    GtkCellRenderer *renderer;
    static GtkTreeViewColumn *column,*column1,*column2,*column3,*column4,*column5;


    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "foreground", "black", NULL);
    column = gtk_tree_view_column_new_with_attributes("name",
            renderer, "text", name_column, NULL);
    gtk_tree_view_column_set_clickable(column,TRUE);
    gtk_tree_view_column_set_resizable(column,TRUE);
    gtk_tree_view_column_set_sort_indicator(column,TRUE);
    gtk_tree_view_column_set_sort_column_id(column,name_column);
    column1= gtk_tree_view_column_new_with_attributes("pid",
            renderer, "text", pid_column, NULL);
    gtk_tree_view_column_set_clickable(column1,TRUE);
    gtk_tree_view_column_set_resizable(column1,TRUE);
    gtk_tree_view_column_set_sort_indicator(column1,TRUE);
    gtk_tree_view_column_set_sort_column_id(column1,pid_column);   
    column2= gtk_tree_view_column_new_with_attributes("ppid",
            renderer, "text", ppid_column, NULL);
    gtk_tree_view_column_set_clickable(column2,TRUE);
    gtk_tree_view_column_set_resizable(column2,TRUE);
    gtk_tree_view_column_set_sort_indicator(column2,TRUE);
    gtk_tree_view_column_set_sort_column_id(column2,ppid_column);
    column3= gtk_tree_view_column_new_with_attributes("state",
            renderer, "text", state_column, NULL);
    gtk_tree_view_column_set_clickable(column3,TRUE);
    gtk_tree_view_column_set_resizable(column3,TRUE);
    gtk_tree_view_column_set_sort_indicator(column3,TRUE);
    gtk_tree_view_column_set_sort_column_id(column3,state_column);
    column4= gtk_tree_view_column_new_with_attributes("mem",
            renderer, "text", mem_column, NULL);
    gtk_tree_view_column_set_clickable(column4,TRUE);
    gtk_tree_view_column_set_resizable(column4,TRUE);
    gtk_tree_view_column_set_sort_indicator(column4,TRUE);
    gtk_tree_view_column_set_sort_column_id(column4,mem_column);
    column5= gtk_tree_view_column_new_with_attributes("prio",
            renderer, "text", prio_column, NULL);
    gtk_tree_view_column_set_clickable(column5,TRUE);
    gtk_tree_view_column_set_resizable(column5,TRUE);
    gtk_tree_view_column_set_sort_column_id(column5,prio_column);
    gtk_tree_view_column_set_sort_indicator(column5,TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column2);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column3);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column4);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column5);
    store = gtk_list_store_new(total_columns, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    

    g_signal_connect(column, "clicked", G_CALLBACK(column_sort),store);
    g_signal_connect(column1, "clicked", G_CALLBACK(column_sort),store);
    g_signal_connect(column2, "clicked", G_CALLBACK(column_sort),store);
    g_signal_connect(column3, "clicked", G_CALLBACK(column_sort),store);
    g_signal_connect(column4, "clicked", G_CALLBACK(column_sort),store);
    g_signal_connect(column5, "clicked", G_CALLBACK(column_sort),store);

    gtk_tree_view_set_model(GTK_TREE_VIEW(list), 
        GTK_TREE_MODEL(store));

    g_object_unref(store);
}

void add_to_list(GtkWidget *gtk_list){
    GtkListStore *store;
    GtkTreeIter iter;
    int i=0,total;
    store = GTK_LIST_STORE(gtk_tree_view_get_model
        (GTK_TREE_VIEW(gtk_list)));
    gtk_list_store_clear(store);
    total=get_proc();
    while(i<total){
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, name_column, list[i].name,
    pid_column,list[i].pid,
    ppid_column,list[i].ppid,
    state_column,list[i].state,
    mem_column,list[i].mem,
    prio_column,list[i].prio,
    -1);
    i++;
    }
}

GtkWidget *Main_Window(void){
    GtkWidget *main_window;
    GtkWidget *main_vbox;
    GtkWidget *menubar_top;
    GtkWidget *menuitem1;
    GtkWidget *menuitem2;
    GtkWidget *menuitem3;
    GtkWidget *submenu2;
    GtkWidget *submenu3;    
    GtkWidget *menu2_1;
    GtkWidget *menu2_2;
    GtkWidget *menu3_1;
    GtkWidget *menu3_2;
    GtkWidget *main_notebook;

    GtkWidget *now_time;
    GtkWidget *cpu_util;
    GtkWidget *mem_util;   

    GtkWidget *page1;
    GtkWidget *page2;
    GtkWidget *page3;
    GtkWidget *frame1_1;
    GtkWidget *text1_1;
    GtkWidget *frame1_2;
    GtkWidget *text1_2;
    GtkWidget *frame1_3;
    GtkWidget *text1_3;
    GtkWidget *frame3_1;
    GtkWidget *frame3_2;
    GtkWidget *frame3_3;
    GtkWidget *table_label1;
    GtkWidget *table_label2;
    GtkWidget *table_label3;
    GtkWidget *scrolled_window1;
    GtkWidget *tree_view1;
    GtkWidget *fresh;
    GtkWidget *delete;
    GtkWidget *fixed;
    GtkWidget *draw_area1;
    GtkWidget *draw_area2;
    GtkWidget *draw_area3;
    GtkTreeSelection *selection;

    main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window),"系统监视器");
    gtk_window_set_default_size(GTK_WINDOW(main_window),600,500);
    g_signal_connect(G_OBJECT(main_window),"delete_event",G_CALLBACK(gtk_main_quit),NULL);
    
    main_vbox=gtk_vbox_new(FALSE,0);
    gtk_widget_show(main_vbox);
    gtk_container_add(GTK_CONTAINER(main_window),main_vbox);

    //创建top_menu
    menubar_top=gtk_menu_bar_new();
    gtk_widget_show(menubar_top);
    gtk_box_pack_start(GTK_BOX(main_vbox),menubar_top,FALSE,FALSE,0);
    menuitem1=gtk_menu_item_new_with_mnemonic("文件");
    gtk_widget_show(menuitem1);
    gtk_menu_bar_append(GTK_MENU_BAR(menubar_top),menuitem1);
    menuitem2=gtk_menu_item_new_with_mnemonic("关于");
    gtk_widget_show(menuitem2);
    gtk_menu_bar_append(GTK_MENU_BAR(menubar_top),menuitem2);
    submenu2=gtk_menu_new();
    gtk_widget_show(submenu2);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem2),submenu2);
    menu2_1=gtk_menu_item_new_with_mnemonic("帮助");
    gtk_widget_show(menu2_1);
    gtk_menu_append(GTK_MENU(submenu2),menu2_1);
    menu2_2=gtk_menu_item_new_with_mnemonic("关于系统管理器");
    gtk_widget_show(menu2_2);
    gtk_menu_append(GTK_MENU(submenu2),menu2_2);
    //menu2_2=gtk_menu_new_with_mnemonic("关于系统管理器");
    menuitem3=gtk_menu_item_new_with_mnemonic("电源");
    gtk_widget_show(menuitem3);
    gtk_menu_bar_append(GTK_MENU_BAR(menubar_top),menuitem3);
    submenu3=gtk_menu_new();
    gtk_widget_show(submenu3);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem3),submenu3);
    menu3_1=gtk_menu_item_new_with_label("重启");
    g_signal_connect_swapped(GTK_MENU_ITEM(menu3_1), "activate", G_CALLBACK (call_reboot),NULL);
    gtk_widget_show(menu3_1);
    gtk_menu_append(GTK_MENU(submenu3),menu3_1); 
    //menu3_1=gtk_menu_new_with_mnemonic("重启");
    menu3_2=gtk_menu_item_new_with_label("关机");
    g_signal_connect_swapped(GTK_MENU_ITEM(menu3_2), "activate", G_CALLBACK (call_shutdown),NULL);
    gtk_widget_show(menu3_2);
    gtk_menu_append(GTK_MENU(submenu3),menu3_2); 
    
    //menu3_2=gtk_menu_new_with_mnemonic("关机");
    //创建notebook
    main_notebook=gtk_notebook_new();
    gtk_widget_show(main_notebook);
    gtk_box_pack_start(GTK_BOX(main_vbox),main_notebook,FALSE,FALSE,0);
    
    page1=gtk_vbox_new(FALSE,0);
    gtk_widget_show(page1);
    table_label1=gtk_label_new("计算机信息");
    gtk_notebook_append_page(GTK_NOTEBOOK(main_notebook),page1,table_label1);

    frame1_1=gtk_frame_new("系统信息");
    gtk_frame_set_shadow_type(GTK_FRAME(frame1_1),GTK_SHADOW_IN);
    gtk_widget_show(frame1_1);
    gtk_box_pack_start(GTK_BOX(page1),frame1_1,FALSE,FALSE,0);
    text1_1=gtk_label_new(NULL);
    gtk_widget_show(text1_1);
    show_sys(text1_1);
    gtk_container_add(GTK_CONTAINER(frame1_1),text1_1);

    frame1_2=gtk_frame_new("处理器信息");
    gtk_frame_set_shadow_type(GTK_FRAME(frame1_2),GTK_SHADOW_IN);
    gtk_widget_show(frame1_2);
    gtk_box_pack_start(GTK_BOX(page1),frame1_2,FALSE,FALSE,0);
    text1_2=gtk_label_new(NULL);
    gtk_widget_show(text1_2);
    show_cpu(text1_2);
    gtk_container_add(GTK_CONTAINER(frame1_2),text1_2);

    frame1_3=gtk_frame_new("About");
    gtk_frame_set_shadow_type(GTK_FRAME(frame1_3),GTK_SHADOW_IN);
    gtk_widget_show(frame1_3);
    gtk_box_pack_start(GTK_BOX(page1),frame1_3,FALSE,FALSE,0);
    text1_3=gtk_label_new(NULL);
    gtk_widget_show(text1_3);
    show_sys_time(text1_3);
    gint show_sys_time_r = g_timeout_add (1000, show_sys_time, (gpointer)text1_3);
    gtk_container_add(GTK_CONTAINER(frame1_3),text1_3);
    
    //显示时间
    now_time = gtk_label_new (NULL);
    gtk_widget_show(now_time);   
    gint settime_r = g_timeout_add (1000, settime, (gpointer)now_time);
    gtk_box_pack_start(GTK_BOX(main_vbox),now_time,FALSE,FALSE,0);
    //显示CPU利用率
    cpu_util = gtk_label_new (NULL);
    gtk_widget_show(cpu_util);   
    gint cal_cpu_util_r = g_timeout_add (1000, cal_cpu_util, (gpointer)cpu_util);
    gtk_box_pack_start(GTK_BOX(main_vbox),cpu_util,FALSE,FALSE,0);
    //显示mem与swap利用率
    mem_util = gtk_label_new (NULL);
    gtk_widget_show(mem_util);   
    gint cal_mem_util_r = g_timeout_add (1000, cal_mem_util, (gpointer)mem_util);
    gtk_box_pack_start(GTK_BOX(main_vbox),mem_util,FALSE,FALSE,0);    


    page2=gtk_hbox_new(FALSE,0);
    gtk_widget_show(page2);
    table_label2=gtk_label_new("进程信息");
    gtk_notebook_append_page(GTK_NOTEBOOK(main_notebook),page2,table_label2);
 

    scrolled_window1=gtk_scrolled_window_new(NULL,NULL);
    gtk_widget_show(scrolled_window1);
    gtk_box_pack_start(GTK_BOX(page2),scrolled_window1, TRUE,TRUE,0);
    gtk_scrolled_window_unset_placement (GTK_SCROLLED_WINDOW(scrolled_window1));
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window1),GTK_SHADOW_IN);
    

    tree_view1=gtk_tree_view_new();
    gtk_widget_show(tree_view1);
    gtk_container_add(GTK_CONTAINER(scrolled_window1),tree_view1);

    init_list(tree_view1);
    add_to_list(tree_view1);

    fresh = gtk_button_new_with_label("刷新");
    gtk_widget_show(fresh);
//    gtk_box_pack_start(GTK_BOX(page2), fresh, FALSE, FALSE, 3);
    g_signal_connect(fresh, "clicked", G_CALLBACK(call_fresh),tree_view1);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view1));


    delete = gtk_button_new_with_label("结束进程");
    gtk_widget_show(delete);
//   gtk_box_pack_start(GTK_BOX(page2), delete, FALSE, FALSE, 3);
    g_signal_connect(delete, "clicked", G_CALLBACK(call_delete),selection);
 
    fixed = gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(fixed), fresh, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), delete,0, 40);
    gtk_widget_show(fixed);
    gtk_box_pack_start(GTK_BOX(page2), fixed, FALSE, FALSE, 5);

   page3=gtk_vbox_new(FALSE,0);
    gtk_widget_show(page3);
    table_label3=gtk_label_new("性能");
    gtk_notebook_append_page(GTK_NOTEBOOK(main_notebook),page3,table_label3);

    frame3_1=gtk_frame_new("cpu使用曲线");
    gtk_container_set_border_width(GTK_CONTAINER(frame3_1),5);
    gtk_widget_set_size_request(frame3_1,200,150);
    gtk_frame_set_shadow_type(GTK_FRAME(frame3_1),GTK_SHADOW_IN);
    gtk_widget_show(frame3_1);
    gtk_box_pack_start(GTK_BOX(page3),frame3_1,FALSE,FALSE,0);
    //绘图
    draw_area1=gtk_drawing_area_new();
    gtk_widget_show(draw_area1);
    gtk_container_add(GTK_CONTAINER(frame3_1),draw_area1);
    g_signal_connect(G_OBJECT(draw_area1),"expose_event",G_CALLBACK(draw_expose),NULL);
    g_signal_connect(G_OBJECT(draw_area1),"configure_event",G_CALLBACK(draw_conf),NULL);

    frame3_2=gtk_frame_new("mem使用曲线");
    gtk_container_set_border_width(GTK_CONTAINER(frame3_2),5);
    gtk_widget_set_size_request(frame3_2,200,150);
    gtk_frame_set_shadow_type(GTK_FRAME(frame3_2),GTK_SHADOW_IN);
    gtk_widget_show(frame3_2);
    gtk_box_pack_start(GTK_BOX(page3),frame3_2,FALSE,FALSE,0);
    //绘图
    draw_area2=gtk_drawing_area_new();
    gtk_widget_show(draw_area2);
    gtk_container_add(GTK_CONTAINER(frame3_2),draw_area2);
    g_signal_connect(G_OBJECT(draw_area2),"expose_event",G_CALLBACK(draw_expose2),NULL);
    g_signal_connect(G_OBJECT(draw_area2),"configure_event",G_CALLBACK(draw_conf2),NULL);

    frame3_3=gtk_frame_new("swap使用曲线");
    gtk_container_set_border_width(GTK_CONTAINER(frame3_3),5);
    gtk_widget_set_size_request(frame3_3,200,150);
    gtk_frame_set_shadow_type(GTK_FRAME(frame3_3),GTK_SHADOW_IN);
    gtk_widget_show(frame3_3);
    gtk_box_pack_start(GTK_BOX(page3),frame3_3,FALSE,FALSE,0);
    //绘图
    draw_area3=gtk_drawing_area_new();
    gtk_widget_show(draw_area3);
    gtk_container_add(GTK_CONTAINER(frame3_3),draw_area3);
    g_signal_connect(G_OBJECT(draw_area3),"expose_event",G_CALLBACK(draw_expose3),NULL);
    g_signal_connect(G_OBJECT(draw_area3),"configure_event",G_CALLBACK(draw_conf3),NULL);

    // gtk_widget_set_events(draw_area1,GDK_EXPOSURE_MASK);
    // gtk_widget_set_events(draw_area2,GDK_EXPOSURE_MASK);
    // gtk_widget_set_events(draw_area3,GDK_EXPOSURE_MASK);

    return main_window;
}

int main(int argc,char *argv[])
{
    GtkWidget *window;
    gtk_set_locale();
    gtk_init(&argc,&argv);
    window=Main_Window();
    gtk_widget_show(window);
    gtk_main();
    return 0;
}

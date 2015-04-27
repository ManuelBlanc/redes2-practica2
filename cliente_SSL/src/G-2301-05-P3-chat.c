
/* std */
#include <stdlib.h>
#include <string.h>
/* redes2 */
#include <redes2/chat.h>
/* ssl */
#include "G-2301-05-P3-ssl_functions.h"
#include "G-2301-05-P3-ssock.h"


#define MAX_PAGES (16)


/* Variables globales */
static GtkWidget *window;
static GtkWidget *eApodo, *eNombre, *eNombreR, *eServidor, *ePuerto;
static GtkTextIter iter[MAX_PAGES];
static GtkTextBuffer *buffer[MAX_PAGES];
static GtkWidget *topic_protectB, *extern_msgB, *secretB, *guestsB, *privatedB, *moderatedB, *notebook;
static gboolean states[MAX_PAGES][6];

static int maxpages = 0;

static void    	scrolling      	(GtkWidget *widget, gpointer data);
static void    	connectCB      	(GtkButton *button, gpointer user_data);
static void    	disconnectCB   	(GtkButton *button, gpointer user_data);
static gboolean	topic_protectCB	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static gboolean	extern_msgCB   	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static gboolean	secretCB       	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static gboolean	guestsCB       	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static gboolean	privatedCB     	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static gboolean	moderatedCB    	(GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data);
static void    	readMessageCB  	(GtkWidget *msg, gpointer user_data);
static void    	pageShowed     	(GtkWidget *scr, gpointer user_data);
static void    	ConnectArea    	(GtkWidget *vbox);
static void    	StateArea      	(GtkWidget *vbox);
static void    	ChatArea       	(GtkWidget *vbox);
static void    	exitCB         	(GtkWidget *wid, gpointer user_data);


gboolean toggleButtonState(GtkToggleButton *togglebutton) {
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
}

int current_page() { return gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)); }

void set_current_page(int page) { gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),page); }

char* get_nick()     	{ return (char*) gtk_entry_get_text(GTK_ENTRY( eApodo   	)); 	}
char* get_name()     	{ return (char*) gtk_entry_get_text(GTK_ENTRY( eNombre  	)); 	}
char* get_real_name()	{ return (char*) gtk_entry_get_text(GTK_ENTRY( eNombreR 	)); 	}
char* get_server()   	{ return (char*) gtk_entry_get_text(GTK_ENTRY( eServidor	)); 	}
int   get_port()     	{ return    atoi(gtk_entry_get_text(GTK_ENTRY( ePuerto  	)));	}

void set_nick(char *nick)         	{ gtk_entry_set_text(GTK_ENTRY( eApodo   	), nick    	); }
void set_name(char *name)         	{ gtk_entry_set_text(GTK_ENTRY( eNombre  	), name    	); }
void set_real_name(char *realname)	{ gtk_entry_set_text(GTK_ENTRY( eNombreR 	), realname	); }
void set_server(char *server)     	{ gtk_entry_set_text(GTK_ENTRY( eServidor	), server  	); }
void set_port(int port)
{
	char aux[16];
	snprintf(aux, sizeof aux, "%d", port);
	gtk_entry_set_text(GTK_ENTRY(ePuerto), aux);
}

void error_window(char *msg)
{
	GtkWidget *pError;

	/* Diálogo error envío */
	pError = gtk_message_dialog_new(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"Error:\n%s", msg);

	gtk_dialog_run(GTK_DIALOG(pError));
	gtk_widget_destroy (pError);
}

/*
gboolean confirmation_window(char *msg)
{
	GtkWidget *pDialog;

	// Diálogo error envío
	pDialog = gtk_message_dialog_new(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"%s", msg);

	g_signal_connect(pDialog, "response",
		G_CALLBACK(gtk_widget_destroy),
		pDialog);

	gtk_dialog_run(GTK_DIALOG(pDialog));
	gtk_widget_destroy (pDialog);
}
*/


void public_text(int pagenum, char *user, char *text)
{
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, user, -1, "blue_fg", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, ": ", -1, "blue_fg", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, text, -1, "italic",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, "\n", -1, "italic",  NULL);
}

void private_text(int pagenum, char *user, char *text)
{
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, user, -1, "blue_fg", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, ": ", -1, "blue_fg", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, text, -1, "green_fg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, "\n", -1, "green_fg",  NULL);
}

void error_text(int pagenum, char *errormessage)
{
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, "Error: ", -1, "magenta_fg", "black_bg","italic", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, errormessage, -1, "magenta_fg", "black_bg","italic", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, "\n", -1, "magenta_fg",  NULL);
}

void message_text(int pagenum, char *message)
{
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, message, -1, "magenta_fg", "italic", "bold", "lmarg",  NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer[pagenum], iter+pagenum, "\n", -1, "magenta_fg",  NULL);
}
void message_textf(int pagenum, const char *fmt, ...)
{
	char message[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(message, sizeof message, fmt, ap);
	va_end(ap);
	message_text(pagenum, message);
}

int add_new_page(char *label)
{
	GtkWidget *tab_label;
	GtkWidget *scroll, *view;
	GtkAdjustment *adjustment;
	int numpage;

	if (maxpages >= MAX_PAGES) return -1;

	tab_label = gtk_label_new(label);

	scroll         	= gtk_scrolled_window_new(NULL,NULL);
	numpage        	= gtk_notebook_append_page (GTK_NOTEBOOK(notebook),scroll, tab_label);
	view           	= gtk_text_view_new();
	buffer[numpage]	= gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll),view);

	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
	gtk_widget_set_size_request(scroll,600,360);
	adjustment = (GtkAdjustment *) gtk_adjustment_new(0., 0., 396., 18., 183., 396.);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(scroll),adjustment);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scroll),GTK_CORNER_BOTTOM_LEFT);

	gtk_text_buffer_create_tag(buffer[numpage], "lmarg",     	"left_margin",	5,                 	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "red_fg",    	"foreground", 	"#D9534F",         	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "blue_fg",   	"foreground", 	"#5CB85C",         	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "magenta_fg",	"foreground", 	"#F0AD4E",         	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "black_bg",  	"background", 	"#222222",         	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "green_fg",  	"foreground", 	"#5BC0DE",         	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "italic",    	"style",      	PANGO_STYLE_ITALIC,	NULL);
	gtk_text_buffer_create_tag(buffer[numpage], "bold",      	"weight",     	PANGO_WEIGHT_BOLD, 	NULL);

	gtk_text_buffer_get_iter_at_offset(buffer[numpage], iter+numpage, 0);

	g_signal_connect(G_OBJECT(scroll), "size-allocate", G_CALLBACK(scrolling),  NULL);
	g_signal_connect(G_OBJECT(scroll), "map",           G_CALLBACK(pageShowed), NULL);

	gtk_widget_show_all(notebook);
	++maxpages;
	return (numpage);
}

char *get_name_page(int index)
{
	GtkWidget *pageaux;
	pageaux = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), index);
	return (char *) gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook), pageaux);
}
int get_index_page(char * name)
{
	int i;
	for (i = 0; i < maxpages; ++i) {
		GtkWidget *pageaux = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), i);
		if (!strcmp((char *) gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook), pageaux),name)) break;
	}
	if (i == maxpages) return -1;
	return i;
}

void delete_page(int index)
{
	// Menor igual que 0 para proteger la pagina *System*
	if (index <= 0 || index >= maxpages) return;

	memmove(buffer+index, buffer+index+1, (maxpages-index-1)*(sizeof *buffer));
	memmove(iter  +index, iter  +index+1, (maxpages-index-1)*(sizeof *iter  ));
	memmove(states+index, states+index+1, (maxpages-index-1)*(sizeof *states));
	memset(states+maxpages-1, FALSE, sizeof *states);
	--maxpages;
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), index);
}

int num_opened_pages() {
	return maxpages - 1;
}


void scrolling(GtkWidget *widget, gpointer data)
{
	GtkAdjustment *adjustment;

	adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
	adjustment->value = adjustment->upper;
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(widget),adjustment);
}


void connectCB (GtkButton *button ,gpointer user_data)   	{ connect_client();            	}
void disconnectCB (GtkButton *button, gpointer user_data)	{ disconnect_client();         	}
void exitCB (GtkWidget *obj, gpointer user_data)         	{ exit_func(); gtk_main_quit();	}

/* ==================================================================
        Checkboxes
   ================================================================== */
#define CREATE_CHECKBOX_FUNCS(index, name)                                                   	\
static gboolean name##CB (GtkToggleButton *togglebutton, GdkEvent *event, gpointer user_data)	\
{                                                                                            	\
   if (event->type == GDK_BUTTON_PRESS) {                                                    	\
       name(!states[current_page()][index]);                                                 	\
       return TRUE;                                                                          	\
   }                                                                                         	\
   return FALSE;                                                                             	\
}                                                                                            	\
void set_##name(int pagenum, gboolean state) {                                               	\
    states[pagenum][index] = state;                                                          	\
    if (pagenum == current_page()) {                                                         	\
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(name##B), state);                     	\
    }                                                                                        	\
}                                                                                            	/**/
CREATE_CHECKBOX_FUNCS(0, topic_protect)
CREATE_CHECKBOX_FUNCS(1, extern_msg)
CREATE_CHECKBOX_FUNCS(2, secret)
CREATE_CHECKBOX_FUNCS(3, guests)
CREATE_CHECKBOX_FUNCS(4, privated)
CREATE_CHECKBOX_FUNCS(5, moderated)
#undef CREATE_CHECKBOX_FUNCS

static void readMessageCB (GtkWidget *msg, gpointer user_data)
{
	new_text((char *) gtk_entry_get_text(GTK_ENTRY(msg)));
	gtk_entry_set_text(GTK_ENTRY(msg), "");
}

static void pageShowed (GtkWidget *scr, gpointer user_data) {
	int page = current_page();
	set_topic_protect (page, states[page][0]);
	set_extern_msg    (page, states[page][1]);
	set_secret        (page, states[page][2]);
	set_guests        (page, states[page][3]);
	set_privated      (page, states[page][4]);
	set_moderated     (page, states[page][5]);
}


static void ConnectArea(GtkWidget *vbox)
{
	GtkWidget *table;
	GtkWidget *hb1, *hb2, *hb3, *hb4, *hb5, *hb6;
	GtkWidget *l1, *l2, *l3, *l4, *l5;
	GtkWidget *frm;
	GtkWidget *bt1, *bt2;

	frm = gtk_frame_new("Conexion");
	gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 2);
	table = gtk_table_new(1, 6, FALSE);
	gtk_container_add(GTK_CONTAINER(frm), table);

	hb1 = gtk_hbox_new(FALSE,2);
	hb2 = gtk_hbox_new(FALSE,2);
	hb3 = gtk_hbox_new(FALSE,2);
	hb4 = gtk_hbox_new(FALSE,2);
	hb5 = gtk_hbox_new(FALSE,2);
	hb6 = gtk_hbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table), hb1, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), hb2, 0, 1, 1, 2, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), hb3, 0, 1, 2, 3, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), hb4, 0, 1, 3, 4, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), hb5, 0, 1, 4, 5, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach(GTK_TABLE(table), hb6, 0, 1, 5, 6, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
	l1 = gtk_label_new("Apodo");
	l2 = gtk_label_new("Nombre");
	l3 = gtk_label_new("Nombre real");
	l4 = gtk_label_new("Servidor");
	l5 = gtk_label_new("Puerto");
	eApodo    = gtk_entry_new();
	eNombre   = gtk_entry_new();
	eNombreR  = gtk_entry_new();
	eServidor = gtk_entry_new();
	ePuerto   = gtk_entry_new();
	bt1 = gtk_button_new_with_mnemonic("_Conectar");
	bt2 = gtk_button_new_with_mnemonic("_Desconectar");

	gtk_box_pack_start(GTK_BOX(hb1), l1, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hb2), l2, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hb3), l3, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hb4), l4, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hb5), l5, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hb6), bt1,TRUE , TRUE , 2);

	gtk_box_pack_end(GTK_BOX(hb1), eApodo   ,FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hb2), eNombre  ,FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hb3), eNombreR ,FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hb4), eServidor,FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hb5), ePuerto  ,FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hb6), bt2      ,TRUE , TRUE , 2);

	g_signal_connect(G_OBJECT(bt1), "clicked", G_CALLBACK(connectCB), NULL);
	g_signal_connect(G_OBJECT(bt2), "clicked", G_CALLBACK(disconnectCB), NULL);

}


void StateArea(GtkWidget *vbox)
{
	GtkWidget *vboxi;
	GtkWidget *frm;

	frm = gtk_frame_new("Estado");
	gtk_box_pack_start(GTK_BOX(vbox), frm,FALSE,FALSE,2);
	vboxi = gtk_vbox_new(FALSE,2);
	gtk_container_add(GTK_CONTAINER(frm), vboxi);

	topic_protectB	= gtk_check_button_new_with_mnemonic( "Proteccion de _topico"	);
	extern_msgB   	= gtk_check_button_new_with_mnemonic( "Mensajes exter_nos"   	);
	secretB       	= gtk_check_button_new_with_mnemonic( "_Secreto"             	);
	guestsB       	= gtk_check_button_new_with_mnemonic( "Solo _invitados"      	);
	privatedB     	= gtk_check_button_new_with_mnemonic( "_Privado"             	);
	moderatedB    	= gtk_check_button_new_with_mnemonic( "_Moderado"            	);

	gtk_box_pack_start(GTK_BOX(vboxi), topic_protectB,	TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vboxi), extern_msgB,   	TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vboxi), secretB,       	TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vboxi), guestsB,       	TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vboxi), privatedB,     	TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vboxi), moderatedB,    	TRUE, TRUE, 2);

	g_signal_connect(G_OBJECT( topic_protectB	), "button-press-event", G_CALLBACK( topic_protectCB	), NULL);
	g_signal_connect(G_OBJECT( extern_msgB   	), "button-press-event", G_CALLBACK( extern_msgCB   	), NULL);
	g_signal_connect(G_OBJECT( secretB       	), "button-press-event", G_CALLBACK( secretCB       	), NULL);
	g_signal_connect(G_OBJECT( guestsB       	), "button-press-event", G_CALLBACK( guestsCB       	), NULL);
	g_signal_connect(G_OBJECT( privatedB     	), "button-press-event", G_CALLBACK( privatedCB     	), NULL);
	g_signal_connect(G_OBJECT( moderatedB    	), "button-press-event", G_CALLBACK( moderatedCB    	), NULL);
}




void ChatArea(GtkWidget *vbox)
{
	GtkWidget *hbox;
	GtkWidget *label, *msg;

	label    = gtk_label_new("Mensaje");
	hbox     = gtk_hbox_new(FALSE,2),
	msg      = gtk_entry_new();
	notebook = gtk_notebook_new();

	gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(vbox),   hbox,     FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), label,    FALSE, FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hbox),   msg,      FALSE, FALSE, 2);

	gtk_widget_set_size_request(msg, 600, -1);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	g_signal_connect(G_OBJECT(msg), "activate", G_CALLBACK(readMessageCB), NULL);

	add_new_page("** System **");
}

Redes2_SSL_CTX* contexto;

int main(int argc, char**argv)
{
	//Info para el SSL
	Redes2_SSL_CTX_config conf;
	conf.ca_file = "cert/root.pem";
	conf.ca_path = NULL;
	conf.key_file = "cert/manu.key";
	conf.pem_file = "cert/manu.pem";

	GtkWidget *hboxg, *vbox1,*vbox2;

	memset(states, sizeof states, FALSE);

	gdk_threads_init();
	gdk_threads_enter();
	gtk_init(&argc, &argv); /* Inicia gnome */

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL); /* Ventana principal */
	gtk_window_set_resizable((GtkWindow*) window, FALSE);
	gtk_window_set_icon_from_file((GtkWindow*) window, "res/icon.png", NULL);

	gtk_window_set_title(GTK_WINDOW(window), "IRC Chat"); /* Título ventana principal */
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 350); /* Tamaño ventana principal */
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); /* Posición ventana principal */

	/* Estructura global */
	hboxg = gtk_hbox_new(FALSE, 5);
	vbox1 = gtk_vbox_new(FALSE, 5);
	vbox2 = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(window), hboxg);
	gtk_box_pack_start(GTK_BOX(hboxg), vbox1, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(hboxg), vbox2, FALSE, FALSE, 1);
	ConnectArea(vbox1);
	StateArea(vbox1);
	ChatArea(vbox2);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(exitCB), NULL);

	gtk_widget_show_all(window); /* Presentación de las ventanas */

	/* INICIALIZACION A CASCOPORRO */
	set_nick("Elronda");
	set_name("Adnorle");
	set_real_name("");
	set_port(6667);
	set_server("metis.ii.uam.es");
	inicializar_nivel_SSL();
	contexto = fijar_contexto_SSL(conf);
	/* INICIALIZACION A CASCOPORRO */

	gtk_main(); /* Administración de la interacción */
	gdk_threads_leave(); /* Salida de hilos */

	return 0;
}


int get_or_create_page(char* name)
{
	int i = get_index_page(name);
	if (i != -1) return i;
	return add_new_page(name);
}

char* current_page_name(void)
{
	return get_name_page(current_page());
}

void inputs_set_editable(gboolean state) {
	gtk_entry_set_editable(GTK_ENTRY( eApodo    ), state);
	gtk_entry_set_editable(GTK_ENTRY( eApodo    ), state);
	gtk_entry_set_editable(GTK_ENTRY( eNombre   ), state);
	gtk_entry_set_editable(GTK_ENTRY( eNombreR  ), state);
	gtk_entry_set_editable(GTK_ENTRY( eServidor ), state);
	gtk_entry_set_editable(GTK_ENTRY( ePuerto   ), state);
}

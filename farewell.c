/* farewell - shutdown dialog meant for systemd-operated linuces. Fork of `goodbye`
 * Copyright (C) 2012 Georg Reinke. All rights reserved.
 * Copyright (C) 2015 Alexey Zhikhartsev. All rights reserved.
 *
 * See LICENSE for copying details.
 */

#include <assert.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* The actions. */
enum {
	Reboot = 0,
	Hibernate,
	Shutdown,
	Suspend,
    ActionsCount_
};

/* The names of some things associated whith each button. */
enum {
	Command = 0,
    Arguments,
	Label,
	Icon
};

static const char* names[4][4] = {
	[Shutdown] = {
		[Command] = "systemctl",
        [Arguments] = "poweroff",
		[Label] = "Shutdown",
		[Icon] = "system-shutdown"
	},
	[Reboot] = {
        [Command] = "systemctl",
        [Arguments] = "reboot",
		[Label] = "Reboot", 
		[Icon] = "system-restart"
	},
	[Suspend] = {
        [Command] = "systemctl",
        [Arguments] = "suspend",
		[Label] = "Suspend",
		[Icon] = "system-suspend"
	},
	[Hibernate] = {
        [Command] = "systemctl",
        [Arguments] = "hibernate",
		[Label] = "Hibernate",
		[Icon] = "system-suspend-hibernate"
	}
};

void version(int exit_val) {
	g_printerr("%s %s\n", PROGNAME, VERSION);

	exit(exit_val);
}

void usage(int exit_val) {
	g_printerr("%s - simple shutdown dialog\n", PROGNAME);
	g_printerr("USAGE: %s [OPTION]\n", PROGNAME);
	g_printerr("OPTIONS:\n");
	g_printerr("  -h|--help:    print this help\n");
	g_printerr("  --version:    show version and copyright notice\n");

	exit(exit_val);
}

void execute(const char *command, const char *args)
{
    pid_t pid = fork();
    if(pid == 0) // child
    {
        execlp(command, command, args, NULL);
    }
    else if(pid > 0) // parent
    {
    }
    else if(pid == -1) // fork failed
    {
        perror("fork");
    }
	gtk_main_quit();
}

void handle_clicked(GtkWidget *widget, gpointer data) {
	int action = -1;

	action = GPOINTER_TO_INT(data);
	assert(action >= 0 && action < ActionsCount_);

    execute(names[action][Command], names[action][Arguments]);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch(event->keyval)
    {
        case GDK_KEY_Escape:
            gtk_main_quit();
            break;
        default:
            return FALSE;
    }
    return FALSE;
}

int main(int argc, char *argv[])
{
	GtkWidget *window, *box;
	GtkWidget *buttons[ActionsCount_];
	GtkWidget *icons[ActionsCount_];
    int buttonVisible[ActionsCount_] = {
        [Shutdown] = 1
        , [Reboot] = 1
        , [Suspend] = 1
        , [Hibernate] = 1};

	gtk_init(&argc, &argv);

    while(--argc)
    {
        if(!strcmp(argv[argc], "-h") || !strcmp(argv[argc], "--help"))
        {
            usage(0);
        }
        else if(!strcmp(argv[argc], "--version"))
        {
            version(0);
        }
        else if(!strcmp(argv[argc], "--noreboot"))
        {
            buttonVisible[Reboot] = 0;
        }
        else if(!strcmp(argv[argc], "--nohibernate"))
        {
            buttonVisible[Hibernate] = 0;
        }
        else if(!strcmp(argv[argc], "--noshutdown"))
        {
            buttonVisible[Shutdown] = 0;
        }
        else if(!strcmp(argv[argc], "--nosuspend"))
        {
            buttonVisible[Suspend] = 0;
        }
        else
        {
            usage(1);
        }
    }

    /* curse if nothing is visible */
    int oneVisible = 0;
	for (int i = 0; i < ActionsCount_; ++i)
    {
        if(buttonVisible[i])
        {
            oneVisible = 1;
            break;
        }
    }
    if(!oneVisible)
    {
        g_printerr("At least one button should be visible\n");
        exit(1);
    }

	/* create the window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Farewell");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);

	/* create the buttons and their icons*/
	for (int i = 0; i < ActionsCount_; ++i) {
        if(buttonVisible[i])
        {
            buttons[i] = gtk_button_new_with_label(names[i][Label]);
            gtk_button_set_always_show_image(GTK_BUTTON(buttons[i]), 1);
            icons[i] = gtk_image_new_from_icon_name(names[i][Icon], GTK_ICON_SIZE_BUTTON);
            gtk_button_set_image(GTK_BUTTON(buttons[i]), icons[i]);
        }
	}


	/* create the box */
#ifdef USE_GTK3
	box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
#else
	box = gtk_hbutton_box_new();
#endif /* USE_GTK3 */

	for (int i = 0; i < ActionsCount_; ++i) 
    {
        if(buttonVisible[i])
        {
            gtk_container_add(GTK_CONTAINER(box), buttons[i]);
        }
    }

	gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_CENTER);
	gtk_container_add(GTK_CONTAINER(window), box);

	/* add the signals */
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	for (int i = 0; i < ActionsCount_; ++i)
    {
        if(buttonVisible[i])
        {
            g_signal_connect(buttons[i]
                , "clicked"
                , G_CALLBACK(handle_clicked)
                , GINT_TO_POINTER(i));
        }
    }

    /*add keys signals*/
    g_signal_connect(G_OBJECT(window), "key_press_event",
            G_CALLBACK(on_key_press), NULL);

	/* show window and quit */
	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}

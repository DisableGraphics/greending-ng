#include "glibmm/miscutils.h"
#include "gtkmm/cssprovider.h"
#include "gtkmm/entry.h"
#include "gtkmm/enums.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/hvbox.h"
#include "gtkmm/notebook.h"
#include "gtkmm/targetentry.h"
#include "gtkmm/widget.h"
#include "sigc++/functors/ptr_fun.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <gtkmm.h>
#include <string>
#include <vector>
#include <webkit2/webkit2.h>
#include <iostream>
#include <fstream>
#include <thread>

std::vector<WebKitWebView *> webViews;
std::vector<Gtk::Widget *> webView_widgets;

WebKitUserContentManager * userContent = webkit_user_content_manager_new();
WebKitUserStyleSheet * customSheet = webkit_user_style_sheet_new("conf/userSheet.css", WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, NULL,NULL);

std::string current_dir;
std::string home_page{""};
std::string search_engine;

int current_tab;

std::string substitute_spaces(std::string term)
{
  for(int i{0}; i < term.size(); i++)
  {
    if(term[i] == ' '){
      term[i] = '+';
    }
  }
  return term;
}

std::string getUrl(std::string searchTerm)
{
  if(searchTerm.find("https://") != std::string::npos || searchTerm.find("http://") != std::string::npos)
  {
    return searchTerm;
  }
  else if(searchTerm.find("http") != std::string::npos && searchTerm.find("localhost") != std::string::npos)
  {
    return searchTerm;
  }
  else if(searchTerm.find("localhost") != std::string::npos)
  {
    return "http://" + searchTerm;
  }
  else if(searchTerm.find(".") != std::string::npos)
  {
    return "http://" + searchTerm;
  }
  else if(searchTerm.find(":") != std::string::npos)
  {
    return searchTerm;
  }
  else
  {
    return search_engine +  substitute_spaces(searchTerm);
  }

}

static void on_tab_added(Gtk::Widget* page, guint page_num, Gtk::Notebook * tabs, Gtk::Entry * uriEntry)
{
  tabs->show_all();
  tabs->set_current_page(tabs->get_n_pages() - 1);
  gtk_widget_grab_focus(GTK_WIDGET(webViews[tabs->get_current_page()]));
}
static void on_tab_changed(Gtk::Widget* page, guint page_num, Gtk::Notebook * tabs, Gtk::Entry * uriEntry)
{
  gtk_entry_set_text(uriEntry->gobj(), webkit_web_view_get_uri(webViews[tabs->get_current_page()]));
  current_tab = tabs->get_current_page();
}
static void load_web_page(Gtk::Notebook * tabs, Gtk::Entry * entry)
{
  char  uri[getUrl(entry->get_text()).size()];
  strcpy(uri, getUrl(entry->get_text()).c_str());
  webkit_web_view_load_uri(webViews[tabs->get_current_page()], uri);
}

static void reload(Gtk::Notebook * tabs)
{
  webkit_web_view_reload(webViews[tabs->get_current_page()]);
  gtk_widget_grab_focus(GTK_WIDGET(webViews[tabs->get_current_page()]));
}

static void remove_tab(Gtk::Notebook * tabs)
{
  int index{tabs->get_current_page()};
  tabs->remove_page(tabs->get_current_page());

  webViews.erase(webViews.begin() + index);
  webView_widgets.erase(webView_widgets.begin() + index);
  gtk_widget_grab_focus(GTK_WIDGET(webViews[tabs->get_current_page()]));
}

static void go_back(Gtk::Notebook * tabs)
{
  webkit_web_view_go_back(webViews[tabs->get_current_page()]);
  gtk_widget_grab_focus(GTK_WIDGET(webViews[tabs->get_current_page()]));
}
static void go_forward(Gtk::Notebook * tabs)
{
  webkit_web_view_go_forward(webViews[tabs->get_current_page()]);
  gtk_widget_grab_focus(GTK_WIDGET(webViews[tabs->get_current_page()]));
}

static void on_get_extensions(WebKitWebContext *context, gpointer user_data)
{
  std::string extensionsFolder{current_dir + "/extensions/"};
  char extensionsFolder_charArray[extensionsFolder.size()];

  strcpy(extensionsFolder_charArray, extensionsFolder.c_str());

  webkit_web_context_set_web_extensions_directory(context, extensionsFolder_charArray);
}

static void on_load_changed (WebKitWebView  *web_view, WebKitLoadEvent load_event, GObject * user_data)
{
  switch (load_event) {
    case WEBKIT_LOAD_STARTED:
        /* New load, we have now a provisional URI */
        //provisional_uri = webkit_web_view_get_uri (web_view);
        /* Here we could start a spinner or update the
         * location bar with the provisional URI */
        
          gtk_entry_set_progress_fraction(GTK_ENTRY(user_data), webkit_web_view_get_estimated_load_progress(webViews[current_tab]));
          gtk_entry_set_text(GTK_ENTRY(user_data), webkit_web_view_get_uri(webViews[current_tab]));
        
        
        break;
    case WEBKIT_LOAD_REDIRECTED:
        //redirected_uri = webkit_web_view_get_uri (web_view);
        
          gtk_entry_set_progress_fraction(GTK_ENTRY(user_data), webkit_web_view_get_estimated_load_progress(webViews[current_tab]));
          gtk_entry_set_text(GTK_ENTRY(user_data), webkit_web_view_get_uri(webViews[current_tab]));
        
        break;
    case WEBKIT_LOAD_COMMITTED:
        /* The load is being performed. Current URI is
         * the final one and it won't change unless a new
         * load is requested or a navigation within the
         * same page is performed */
        //uri = webkit_web_view_get_uri (web_view);
        
          gtk_entry_set_progress_fraction(GTK_ENTRY(user_data), webkit_web_view_get_estimated_load_progress(webViews[current_tab]));
        
        break;
    case WEBKIT_LOAD_FINISHED:
        /* Load finished, we can now stop the spinner */
          gtk_entry_set_progress_fraction(GTK_ENTRY(user_data), 0.0);
          gtk_entry_set_text(GTK_ENTRY(user_data), webkit_web_view_get_uri(webViews[current_tab]));
          gtk_widget_grab_focus(GTK_WIDGET(webViews[current_tab]));
        break;
    }
}

bool on_close(GdkEventAny* event, Gtk::Window * window, Glib::RefPtr<Gtk::Application> app, Gtk::Notebook * tabs)
{
  std::string toSave{std::to_string(window->get_width() - 10)  + " " + std::to_string(window->get_height() - 10 - 48)};
  std::ofstream sizeFile;
  sizeFile.open(current_dir + "/conf/size.conf");

  sizeFile << toSave;
  sizeFile.close();

  std::ofstream websFile;
  toSave = "";
  for(auto & web : webViews){
    toSave += webkit_web_view_get_uri(web);
    toSave += "\n";
  }
  websFile.open(current_dir + "/conf/webs.conf");
  websFile << toSave;

  websFile.close();

  std::ofstream activeFile;
  activeFile.open(current_dir + "/conf/active.conf");
  activeFile << tabs->get_current_page();
  activeFile.close();

  app->quit();

  return false;
}

static void new_tab(Gtk::Notebook *tabs, Gtk::Entry *uriEntry)
{

  WebKitSettings * settings = WEBKIT_SETTINGS(webkit_settings_new());
  webkit_settings_set_enable_smooth_scrolling(settings, TRUE);
  webkit_settings_set_enable_webgl(settings, TRUE);

  WebKitWebView * one =  WEBKIT_WEB_VIEW( webkit_web_view_new_with_user_content_manager(userContent) );
  webViews.push_back(one);

  
  /*
   * the next line does some tricks :
   * GTK_WIDGET( one ) -> convert WebKitWebView to GtkWidget (one->two)
   * Glib::wrap( GTK_WIDGET( one ) ) -> convert GtkWidget to Gtk::Widget (two->three)
   */
  Gtk::Widget * webView = Glib::wrap( GTK_WIDGET( one ) );
  webView_widgets.push_back(webView);

  tabs->add(*webView_widgets[tabs->get_n_pages()]);

  char home_page_array[home_page.size()];
  strcpy(home_page_array, home_page.c_str());

  webkit_web_view_load_uri(one, home_page_array);

  webkit_web_view_set_settings(one, settings);

  g_signal_connect_object(one,"load-changed",G_CALLBACK(on_load_changed), uriEntry->gobj(), G_CONNECT_AFTER);
  tabs->show_all();
}

static void initAdblock()
{
  system("adblock-rust-server");
}

int main( int argc, char **argv)
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create( argc, argv, "" );

  webkit_user_content_manager_add_style_sheet(userContent, customSheet);

  Gtk::Window window;

  current_dir = Glib::get_current_dir();

  Gtk::Entry uriEntry;
  Gtk::Button BackB{"←"}, NextB{"→"}, ReloadP{"↻"}, NewP{"+"}, RemoveP{"-"};
  Gtk::HeaderBar headBar;

  headBar.set_border_width(1);
  headBar.set_show_close_button();
  window.set_titlebar(headBar);

  Gtk::VBox uiVbox;
  Gtk::HBox barHBox;

  uriEntry.set_vexpand(false);

  headBar.set_title("Greending-ng");

  WebKitSettings * settings = WEBKIT_SETTINGS(webkit_settings_new());
  webkit_settings_set_enable_smooth_scrolling(settings, TRUE);
  webkit_settings_set_enable_webgl(settings, TRUE);

  Gtk::Grid grid;

  std::ifstream sizeFile;
  sizeFile.open(current_dir + "/conf/size.conf");
  std::string sizeHelper;
  sizeFile >> sizeHelper;
  int width{stoi(sizeHelper)};
  sizeFile >> sizeHelper;
  int height{stoi(sizeHelper)};

  sizeFile.close();

  sizeFile.open(current_dir + "/conf/homePage.conf");
  sizeFile >> home_page;
  sizeFile.close();
  sizeFile.open(current_dir + "/conf/searchEngine.conf");
  sizeFile >> search_engine;
  sizeFile.close();
  
  WebKitWebView * one =  WEBKIT_WEB_VIEW( webkit_web_view_new_with_user_content_manager(userContent) );
  webViews.push_back(one);
  /*
   * the next line does some tricks :
   * GTK_WIDGET( one ) -> convert WebKitWebView to GtkWidget (one->two)
   * Glib::wrap( GTK_WIDGET( one ) ) -> convert GtkWidget to Gtk::Widget (two->three)
   */
  Gtk::Widget * webView = Glib::wrap( GTK_WIDGET( one ) );
  webView_widgets.push_back(webView);

  Gtk::Notebook tabs;
  
  tabs.add(*webView_widgets[tabs.get_n_pages()]);

  uiVbox.pack_end(tabs);
  uiVbox.pack_start(barHBox, false, false);
  barHBox.pack_start(BackB, false, false);
  barHBox.pack_start(NextB, false, false);
  barHBox.pack_start(ReloadP, false, false);
  barHBox.pack_start(uriEntry);
  barHBox.pack_start(NewP, false, false);
  barHBox.pack_start(RemoveP, false, false);
  
  webkit_web_view_set_settings(one, settings);

  uriEntry.signal_activate().connect(sigc::bind(sigc::ptr_fun(&load_web_page), &tabs, &uriEntry));
  BackB.signal_clicked().connect(sigc::bind(sigc::ptr_fun(go_back), &tabs));
  NextB.signal_clicked().connect(sigc::bind(sigc::ptr_fun(go_forward), &tabs));
  NewP.signal_clicked().connect(sigc::bind(sigc::ptr_fun(new_tab), &tabs, &uriEntry));
  RemoveP.signal_clicked().connect(sigc::bind(sigc::ptr_fun(remove_tab), &tabs));
  ReloadP.signal_clicked().connect(sigc::bind(sigc::ptr_fun(reload), &tabs));
  g_signal_connect_object(one,"load-changed",G_CALLBACK(on_load_changed), uriEntry.gobj(), G_CONNECT_AFTER);
  g_signal_connect(webkit_web_context_get_default(), "initialize-web-extensions",G_CALLBACK(on_get_extensions), NULL);
  
  window.signal_delete_event().connect(sigc::bind(sigc::ptr_fun(&on_close), &window, app, &tabs));

  tabs.signal_page_added().connect(sigc::bind(sigc::ptr_fun(on_tab_added), &tabs, &uriEntry));
  tabs.signal_switch_page().connect(sigc::bind(sigc::ptr_fun(on_tab_changed), &tabs, &uriEntry));

  std::ifstream webFile;
  webFile.open(current_dir + "/conf/webs.conf");

  std::string uris;
  webFile >> uris;
  char uris_array[uris.size()];
  strcpy(uris_array, uris.c_str());
  webkit_web_view_load_uri(one, uris_array);

  while(webFile >> uris)
  {
    WebKitWebView * tmp =  WEBKIT_WEB_VIEW( webkit_web_view_new_with_user_content_manager(userContent) );
    webViews.push_back(tmp);

    g_signal_connect_object(tmp,"load-changed",G_CALLBACK(on_load_changed), uriEntry.gobj(), G_CONNECT_AFTER);
    Gtk::Widget * tmp_widget = Glib::wrap( GTK_WIDGET( tmp ) );
    webView_widgets.push_back(tmp_widget);

    tabs.add(*webView_widgets[tabs.get_n_pages()]);

    char uris_array[uris.size()];
    strcpy(uris_array, uris.c_str());
    webkit_web_view_load_uri(tmp, uris_array);
    webkit_web_view_set_settings(tmp, settings);
    g_signal_connect(webkit_web_context_get_default(), "initialize-web-extensions",G_CALLBACK(on_get_extensions), NULL);
  }

  std::ifstream activeFile;
  activeFile.open(current_dir + "/conf/active.conf");
  activeFile >> uris; //Reusing the string

  activeFile.close();
  if(stoi(uris) <= 0)
  {
    tabs.set_current_page(1);
  } 
  else 
  {
    tabs.set_current_page(stoi(uris));
  }
  
  current_tab = tabs.get_current_page();

  window.resize( width, height );

  Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
  if(!css->load_from_path("conf/gtkStyles.css"))
  {
    std::cerr << "Failed to load css: File \"conf/gtkStyles.css\" not found\n";
    std::exit(1);
  }

  auto screen = Gdk::Screen::get_default();

  auto windowContext = window.get_style_context();
  windowContext->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window.add(uiVbox);

  window.show_all();

  std::thread t{initAdblock};
  t.detach();
  
  app->run( window );
  exit( 0 );
}


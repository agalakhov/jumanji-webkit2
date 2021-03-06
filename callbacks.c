/* See LICENSE file for license and copyright information */

#include <girara/session.h>
#include <girara/datastructures.h>
#include <girara/statusbar.h>
#include <girara/tabs.h>
#include <girara/settings.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "database.h"
#include "download.h"
#include "shortcuts.h"
#include "soup.h"
#include "jumanji.h"

gboolean
cb_destroy(GtkWidget* widget, gpointer data)
{
  return TRUE;
}

void
cb_girara_buffer_changed(girara_session_t* session)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(session->global.data != NULL);

  jumanji_t* jumanji = session->global.data;

  char* buffer = girara_buffer_get(session);

  if (buffer) {
    girara_statusbar_item_set_text(session, jumanji->ui.statusbar.buffer, buffer);
    free(buffer);
  } else {
    girara_statusbar_item_set_text(session, jumanji->ui.statusbar.buffer, "");
  }
}

void
cb_jumanji_tab_destroy(GObject* object, jumanji_tab_t* tab)
{
  if (object == NULL || tab == NULL || tab->jumanji == NULL) {
    return;
  }

  /* TODO: We may need to lock jumanji_tab_t instance
   * with either a GRWLock or GRecMutex to avoid races */
  g_signal_handlers_disconnect_by_data(G_OBJECT(tab->web_view), tab);

  if (tab->web_view != NULL && tab->jumanji->global.last_closed != NULL) {
    const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    if (uri != NULL) {
      char* tmp = g_strdup(uri);
      girara_list_prepend(tab->jumanji->global.last_closed, tmp);
    }
  }

  jumanji_tab_free(tab);
}

void
cb_jumanji_tab_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, jumanji_tab_t* tab)
{
  if (web_view == NULL || tab == NULL || tab->jumanji == NULL || tab->jumanji->ui.session == NULL
      || tab->girara_tab == NULL) {
    return;
  }

  const gchar* url   = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  const gchar* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));

  if (load_event == WEBKIT_LOAD_FINISHED) {
    bool enable_private_browsing = true;
    girara_setting_get(tab->jumanji->ui.session, "enable-private-browsing", &enable_private_browsing);
    if (enable_private_browsing == false) {
      jumanji_db_history_add(tab->jumanji->database, url, title);
    }
  }

  unsigned int position = girara_tab_position_get(tab->jumanji->ui.session, tab->girara_tab) + 1;
  title = title ? title : "Loading...";

  char* text = g_strdup_printf("%d | %s", position, title);
  girara_tab_title_set(tab->girara_tab, text);
  g_free(text);

  if (tab == jumanji_tab_get_current(tab->jumanji)) {
    const gchar* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    girara_statusbar_item_set_text(tab->jumanji->ui.session, tab->jumanji->ui.statusbar.url, url ? (char*) url : "Loading...");
  }
}

void
cb_jumanji_tab_changed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji)
{
  if (tabs == NULL || page == NULL || jumanji == NULL || jumanji->ui.session == NULL) {
    return;
  }

  jumanji_tab_t* tab = jumanji_tab_get_nth(jumanji, page_num);

  if (tab == NULL) {
    return;
  }

  if (jumanji->ui.statusbar.url != NULL) {
    const gchar* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.url, url ? (char*) url : "Loading...");
  }

  if (jumanji->ui.statusbar.tabs != NULL) {
    int number_of_tabs = girara_get_number_of_tabs(jumanji->ui.session);
    char* text = g_strdup_printf("[%d/%d]", page_num + 1, number_of_tabs);
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.tabs, text);
    g_free(text);
  }
}

void
cb_jumanji_tab_removed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji)
{
  if (tabs == NULL || jumanji == NULL || jumanji->ui.session == NULL) {
    return;
  }

  if (gtk_notebook_get_n_pages(tabs) == 0) {
    bool close_window_with_last_tab = false;
    girara_setting_get(jumanji->ui.session, "close-window-with-last-tab", &close_window_with_last_tab);
    if (close_window_with_last_tab == true) {
      cb_destroy(NULL, NULL);
      gtk_main_quit();
    } else {
      char* homepage = NULL;
      girara_setting_get(jumanji->ui.session, "homepage", &homepage);
      if (homepage != NULL) {
        char* url = jumanji_build_url_from_string(jumanji, homepage);
        bool focus_new_tabs;
        girara_setting_get(jumanji->ui.session,
                           "focus-new-tabs", &focus_new_tabs);
        jumanji_tab_new(jumanji, url, focus_new_tabs);
        free(url);
      }
      g_free(homepage);
    }
  }
}

void
cb_jumanji_tab_mouse_target_changed(WebKitWebView* web_view,
    WebKitHitTestResult* hit_test_result, guint modifiers,
    jumanji_tab_t* tab)
{
  if (tab == NULL || tab->jumanji == NULL || tab->jumanji->ui.statusbar.url == NULL
      || tab->jumanji->ui.session == NULL) {
    return;
  }

  const gchar* link = webkit_hit_test_result_get_link_uri(hit_test_result);
  if (link != NULL) {
    girara_statusbar_item_set_text(tab->jumanji->ui.session, tab->jumanji->ui.statusbar.url, link);
  } else {
    const gchar* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    girara_statusbar_item_set_text(tab->jumanji->ui.session, tab->jumanji->ui.statusbar.url, url ? (char*) url : "Loading...");
  }
}

WebKitWebView*
cb_jumanji_tab_web_inspector(WebKitWebInspector* inspector, WebKitWebView* web_view, gpointer data)
{
  if (web_view == NULL) {
    return NULL;
  }

  GtkWidget* window       = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget* new_web_view = webkit_web_view_new();

  if (window == NULL || new_web_view == NULL) {
    return NULL;
  }

  gchar* title = g_strdup_printf("WebInspector (%s) - jumanji", webkit_web_view_get_uri(web_view));
  if (title != NULL) {
    gtk_window_set_title(GTK_WINDOW(window), title);
    g_free(title);
  }

  gtk_container_add(GTK_CONTAINER(window), new_web_view);
  gtk_widget_show_all(window);

  return WEBKIT_WEB_VIEW(new_web_view);
}

//bool
//cb_jumanji_tab_download_requested(WebKitWebView* web_view, WebKitDownload* download, jumanji_tab_t* tab)
//{
//  if (tab == NULL || tab->jumanji == NULL || download == NULL) {
//    return false;
//  }
//
//  return jumanji_download_file(tab->jumanji, download);
//}

static bool
jumanji_tab_mime_policy(WebKitWebView* web_view,
    WebKitResponsePolicyDecision* decision,
    jumanji_tab_t* tab)
{
  WebKitURIResponse* response = webkit_response_policy_decision_get_response(decision);
  const gchar* mimetype = webkit_uri_response_get_mime_type(response);

  if (webkit_web_view_can_show_mime_type(web_view, mimetype) != TRUE) {
    webkit_policy_decision_download(WEBKIT_POLICY_DECISION(decision));
    return true;
  }

  return false;
}

static bool
jumanji_tab_navigation_policy(WebKitWebView* web_view,
    WebKitNavigationPolicyDecision* decision,
    bool want_new_window,
    jumanji_tab_t* tab)
{
  bool new_tab = false;

  WebKitNavigationType navtype = webkit_navigation_policy_decision_get_navigation_type(decision);
  switch (navtype) {
    case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED:
      if (want_new_window) {
        new_tab = true;
      }
      /* middle mouse button? */
      if (webkit_navigation_policy_decision_get_mouse_button(decision) == 2) {
        new_tab = true;
      }
      break;
    default:
      break;
  }

  WebKitURIRequest* request = webkit_navigation_policy_decision_get_request(decision);
  const char* uri = webkit_uri_request_get_uri(request);
  if (uri == NULL) {
    return false;
  }

  if (new_tab) {
    bool focus_new_tabs;
    girara_setting_get(tab->jumanji->ui.session, "focus-new-tabs", &focus_new_tabs);
    jumanji_tab_new(tab->jumanji, uri, focus_new_tabs);
    webkit_policy_decision_ignore(WEBKIT_POLICY_DECISION(decision));
    return true;
  }
  return false;
}


bool
cb_jumanji_tab_decide_policy(WebKitWebView* web_view,
    WebKitPolicyDecision* decision, WebKitPolicyDecisionType decision_type,
    jumanji_tab_t* tab)
{
  if (web_view == NULL) {
    return false;
  }

  switch(decision_type) {
    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
      return jumanji_tab_navigation_policy(web_view, WEBKIT_NAVIGATION_POLICY_DECISION(decision), true, tab);
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
      return jumanji_tab_mime_policy(web_view, WEBKIT_RESPONSE_POLICY_DECISION(decision), tab);
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
      return jumanji_tab_navigation_policy(web_view, WEBKIT_NAVIGATION_POLICY_DECISION(decision), false, tab);
  }

  return false;
}


void
cb_settings_webkit(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(value != NULL);
  g_return_if_fail(session->global.data != NULL);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  WebKitSettings* browser_settings = NULL;
  jumanji_tab_t* tab                  = jumanji_tab_get_current(jumanji);

  /* get settings */
  if (girara_get_number_of_tabs(session) == 0) {
    browser_settings = jumanji->global.browser_settings;
  } else if (tab != NULL) {
    browser_settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(tab->web_view));
  } else {
    return;
  }

  /* special case: set value in webkitview */
  if (g_strcmp0(name, "full-content-zoom") == 0) {
    if (tab && tab->web_view) {
      g_object_set(G_OBJECT(tab->web_view), name, *(bool*)value, NULL);
    }
  } else if (browser_settings != NULL) {
    switch (type) {
      case STRING:
        g_object_set(G_OBJECT(browser_settings), name, (const char*) value, NULL);
        break;
      case INT:
        g_object_set(G_OBJECT(browser_settings), name, *(int*) value, NULL);
        break;
      case FLOAT:
        g_object_set(G_OBJECT(browser_settings), name, *(float*) value, NULL);
        break;
      case BOOLEAN:
        g_object_set(G_OBJECT(browser_settings), name, *(bool*) value, NULL);
        break;
      default:
        return;
    }

    /* reload website */
    girara_argument_t argument = { 0, NULL };
    sc_reload(session, &argument, NULL, 0);
  }
}

bool
cb_statusbar_proxy(GtkWidget* widget, GdkEvent* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  if (jumanji->global.soup == NULL || jumanji->global.proxies == NULL) {
    return false;
  }

  int number_of_proxies = girara_list_size(jumanji->global.proxies);

  if (number_of_proxies == 0) {
    return false;
  }

  jumanji_proxy_t* proxy = NULL;

  if (jumanji->global.current_proxy == NULL) {
    proxy = (jumanji_proxy_t*) girara_list_nth(jumanji->global.proxies, 0);
  } else {
    int current_proxy = girara_list_position(jumanji->global.proxies, jumanji->global.current_proxy);

    if (current_proxy != (number_of_proxies - 1)) {
      proxy = (jumanji_proxy_t*) girara_list_nth(jumanji->global.proxies, (current_proxy + 1) % number_of_proxies);
    }
  }

  jumanji_proxy_set(jumanji, proxy);

  return true;
}

/* See LICENSE file for license and copyright information */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include <girara/types.h>

#include "jumanji.h"

/**
 * Quits the current jumanji session
 *
 * @param widget The gtk window of jumanji
 * @param data NULL
 * @return TRUE
 */
gboolean cb_destroy(GtkWidget* widget, gpointer data);

/**
 * This function gets called when the buffer of girara changes
 *
 * @param session The girara session
 */
void cb_girara_buffer_changed(girara_session_t* session);

/**
 * Called when a jumanji tab is destroyed
 *
 * @param object The tab scrolld window widget
 * @param tab The jumanji tab
 */
void cb_jumanji_tab_destroy(GObject* object, jumanji_tab_t* tab);

/**
 * Update the title of the tab if status changes
 *
 * @param web_view Webkit web view
 * @param pspec -
 * @param data Custom data
 */
void cb_jumanji_tab_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, jumanji_tab_t* tab);

/**
 * Updates the statusbar entry
 *
 * @param tabs Tab container
 * @param page Current tab
 * @param page_num Current tab id
 * @param jumanji The jumanji session
 */
void cb_jumanji_tab_changed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji);

/**
 * Tab has been removed
 *
 * @param tabs Tab container
 * @param page Current tab
 * @param page_num Current tab id
 * @param jumanji The jumanji session
 */
void cb_jumanji_tab_removed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji);

/**
 * Update link in statusbar if link has been hovered
 *
 * @param web_view The web view
 * @param title The links title
 * @param link The uri the link points to
 * @param tab The jumanji tab
 */
void cb_jumanji_tab_mouse_target_changed(WebKitWebView* web_view,
    WebKitHitTestResult* hit_test_result, guint modifiers,
    jumanji_tab_t* tab);

/**
 * Inspect web view
 *
 * @param inspector The web inspector object
 * @param web_view The web view
 * @param data Custom data
 * @return webkit web view or NULL if an error occured
 */
//WebKitWebView* cb_jumanji_tab_web_inspector(WebKitWebInspector* inspector, WebKitWebView* web_view, gpointer data);

/**
 * Download the file if webkit is not able to display it
 *
 * @param web_view The web view
 * @param frame The frame
 * @param request Request
 * @param mimetype Mime type
 * @param decision Policy Decision
 * @param tab Jumanji tab
 * @return true if request is handled
 */
bool cb_jumanji_tab_decide_policy(WebKitWebView* web_view,
    WebKitPolicyDecision* decision, WebKitPolicyDecisionType decision_type,
    jumanji_tab_t* tab);

/**
 * Called when a webkit setting has been changed
 *
 * @param session The girara session
 * @param setting The girara setting
 */
void cb_settings_webkit(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data);

/**
 * Executed when someone clicks the statusbar entry
 *
 * @param widget The statusbar
 * @param event Occured event
 * @param session The girara session
 * @return true if no error occured
 */
bool cb_statusbar_proxy(GtkWidget* widget, GdkEvent* event, girara_session_t* session);

#endif // CALLBACKS_H

/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara.h>

#include "database.h"
#include "soup.h"

jumanji_soup_t*
jumanji_soup_init(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->config.config_dir == NULL) {
    return NULL;
  }

  jumanji_soup_t* soup = malloc(sizeof(jumanji_soup_t));
  if (soup == NULL) {
    return NULL;
  }

  /* libsoup */
  soup->session = webkit_get_default_session();
  if (soup->session == NULL) {
    free(soup);
    return NULL;
  }

  g_signal_connect(G_OBJECT(soup->session), "request-started",
      G_CALLBACK(cb_jumanji_soup_session_request_started), jumanji);

  soup->cookie_jar = soup_cookie_jar_new();
  if (soup->cookie_jar == NULL) {
    free(soup);
    return NULL;
  }

  g_signal_connect(G_OBJECT(soup->cookie_jar), "changed",
      G_CALLBACK(cb_jumanji_soup_jar_changed), jumanji);

  soup_session_add_feature(soup->session, (SoupSessionFeature*) soup->cookie_jar);

  return soup;
}

void
jumanji_soup_free(jumanji_soup_t* soup)
{
  if (soup == NULL) {
    return;
  }

  free(soup);
}

void
cb_jumanji_soup_jar_changed(SoupCookieJar* jar, SoupCookie* old_cookie,
    SoupCookie* new_cookie, jumanji_t* jumanji)
{
  if (jar == NULL || jumanji == NULL || jumanji->database == NULL) {
    return;
  }

  SoupCookie* cookie = (new_cookie != NULL) ? new_cookie :
    ((old_cookie != NULL) ? old_cookie : NULL);
  if (cookie == NULL) {
    return;
  }

  const char* name   = soup_cookie_get_name(cookie);
  const char* value  = soup_cookie_get_value(cookie);
  const char* domain = soup_cookie_get_domain(cookie);
  const char* path   = soup_cookie_get_path(cookie);
  SoupDate* expires  = soup_cookie_get_expires(cookie);
  gboolean secure    = soup_cookie_get_secure(cookie);
  gboolean http_only = soup_cookie_get_http_only(cookie);

  if (new_cookie != NULL) {
    jumanji_db_cookie_add(jumanji->database, name, value, domain, path,
        soup_date_to_time_t(expires), (secure == TRUE) ? true : false,
        (http_only == TRUE) ? true : false);
  } else {
    jumanji_db_cookie_remove(jumanji->database, domain, name);
  }
}

void
cb_jumanji_soup_session_request_started(SoupSession* soup_session, SoupMessage*
    message, SoupSocket* socket, jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->database== NULL) {
    return;
  }

  /* read cookies */
  girara_list_t* cookies = jumanji_db_cookie_list(jumanji->database);
  if (cookies != NULL) {
    jumanji_soup_add_cookies(jumanji, cookies);
    girara_list_free(cookies);
  }

  SoupCookieJar* cookie_jar = ((jumanji_soup_t*)jumanji->global.soup)->cookie_jar;
  SoupURI* uri              = soup_message_get_uri(message);
  gchar* header             = soup_cookie_jar_get_cookies(cookie_jar, uri, TRUE);

  if (header != NULL) {
    soup_message_headers_replace(message->request_headers, "Cookie", header);
    g_free(header);
  } else {
    soup_message_headers_remove(message->request_headers, "Cookie");
  }
}

void
jumanji_soup_add_cookies(jumanji_t* jumanji, girara_list_t* cookies)
{
  if (jumanji == NULL || cookies == NULL || jumanji->global.soup == 0 ||
      girara_list_size(cookies) == 0) {
    return;
  }

  jumanji_soup_t* soup = (jumanji_soup_t*) jumanji->global.soup;

  if (soup->cookie_jar == NULL) {
    return;
  }

  girara_list_iterator_t* iter = girara_list_iterator(cookies);
  do {
    SoupCookie* cookie = girara_list_iterator_data(iter);
    soup_cookie_jar_add_cookie(soup->cookie_jar, cookie);
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);
}

void
jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy)
{
  if (jumanji == NULL || jumanji->global.soup == NULL) {
    return;
  }

  jumanji_soup_t* soup = (jumanji_soup_t*) jumanji->global.soup;

  if (proxy != NULL && proxy->url != NULL) {
    SoupURI* soup_uri = soup_uri_new(proxy->url);
    g_object_set(soup->session, "proxy-uri", soup_uri, NULL);
    soup_uri_free(soup_uri);
    jumanji->global.current_proxy = proxy;

    char* text = (proxy->description != NULL) ? proxy->description : proxy->url;
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, text);
  } else {
    g_object_set(soup->session, "proxy-uri", NULL, NULL);
    jumanji->global.current_proxy = NULL;

    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, "Proxy disabled");
  }
}

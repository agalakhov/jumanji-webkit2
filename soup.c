/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/girara.h>

#include <webkit2/webkit2.h>

#include "soup.h"

struct jumanji_soup_s
{
  WebKitWebContext* web_context;
  WebKitCookieManager* cookie_manager;
};

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

  soup->web_context= webkit_web_context_get_default();
  if (soup->web_context == NULL) {
    free(soup);
    return NULL;
  }

  soup->cookie_manager = webkit_web_context_get_cookie_manager(soup->web_context);
  if (soup->cookie_manager == NULL) {
    free(soup);
    return NULL;
  }

  char* cookie_file = g_build_filename(jumanji->config.config_dir,
      JUMANJI_COOKIE_FILE, NULL);
  if (cookie_file == NULL) {
    free(soup);
    return NULL;
  }

  webkit_cookie_manager_set_persistent_storage(soup->cookie_manager, cookie_file,
      WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT);

  g_free(cookie_file);

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
jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy)
{
  if (jumanji == NULL || jumanji->global.soup == NULL) {
    return;
  }

  jumanji_soup_t* soup = (jumanji_soup_t*) jumanji->global.soup;

  if (proxy != NULL && proxy->url != NULL) {
    SoupURI* soup_uri = soup_uri_new(proxy->url);
    //g_object_set(soup->session, "proxy-uri", soup_uri, NULL);
    soup_uri_free(soup_uri);
    jumanji->global.current_proxy = proxy;

    char* text = (proxy->description != NULL) ? proxy->description : proxy->url;
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, text);
  } else {
    //g_object_set(soup->session, "proxy-uri", NULL, NULL);
    jumanji->global.current_proxy = NULL;

    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, "Proxy disabled");
  }
}

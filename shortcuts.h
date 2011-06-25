/* See LICENSE file for license and copyright information */

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <girara.h>

/**
 * Focus the inputbar
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Follows a link
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_follow_link(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Navigates through the tabs history
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_navigate_history(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Quit jumanji
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quit(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Reloads the current page
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_reload(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Scroll through the web site
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_scroll(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Toggle between the rendered and source code view
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_source_mode(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Zoom in/out
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_zoom(girara_session_t* session, girara_argument_t* argument, unsigned int t);

#endif // SHORTCUTS_H

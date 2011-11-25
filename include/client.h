
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libguac.
 *
 * The Initial Developer of the Original Code is
 * Michael Jumper.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#ifndef _GUAC_CLIENT_H
#define _GUAC_CLIENT_H

#include <stdarg.h>

#include "socket.h"
#include "protocol.h"

/**
 * Provides functions and structures required for defining (and handling) a proxy client.
 *
 * @file client.h
 */

typedef struct guac_client guac_client;

/**
 * Handler for server messages (where "server" refers to the server that
 * the proxy client is connected to).
 */
typedef int guac_client_handle_messages(guac_client* client);

/**
 * Handler for Guacamole mouse events.
 */
typedef int guac_client_mouse_handler(guac_client* client, int x, int y, int button_mask);

/**
 * Handler for Guacamole key events.
 */
typedef int guac_client_key_handler(guac_client* client, int keysym, int pressed);

/**
 * Handler for Guacamole clipboard events.
 */
typedef int guac_client_clipboard_handler(guac_client* client, char* copied);

/**
 * Handler for freeing up any extra data allocated by the client
 * implementation.
 */
typedef int guac_client_free_handler(guac_client* client);

/**
 * Handler for logging messages
 */
typedef void guac_client_log_handler(guac_client* client, const char* format, va_list args); 

/**
 * Possible current states of the Guacamole client. Currently, the only
 * two states are RUNNING and STOPPING.
 */
typedef enum guac_client_state {

    /**
     * The state of the client from when it has been allocated by the main
     * daemon until it is killed or disconnected.
     */
    RUNNING,

    /**
     * The state of the client when a stop has been requested, signalling the
     * I/O threads to shutdown.
     */
    STOPPING

} guac_client_state;

/**
 * Guacamole proxy client.
 *
 * Represents a Guacamole proxy client (the client which communicates to
 * a server on behalf of Guacamole, on behalf of the web-client).
 */
struct guac_client {

    /**
     * The guac_socket structure to be used to communicate with the web-client. It is
     * expected that the implementor of any Guacamole proxy client will provide
     * their own mechanism of I/O for their protocol. The guac_socket structure is
     * used only to communicate conveniently with the Guacamole web-client.
     */
    guac_socket* io;

    /**
     * The current state of the client. When the client is first allocated,
     * this will be initialized to RUNNING. It will remain at RUNNING until
     * an event occurs which requires the client to shutdown, at which point
     * the state becomes STOPPING.
     */
    guac_client_state state;

    /**
     * The index of the next available buffer.
     */
    int __next_buffer_index;

    /**
     * The head pointer of the list of all available (allocated but not used)
     * buffers.
     */
    guac_layer* __available_buffers;

    /**
     * The head pointer of the list of all allocated layers, regardless of use
     * status.
     */
    guac_layer* __all_layers;

    /**
     * The time (in milliseconds) of receipt of the last sync message from
     * the client.
     */
    guac_timestamp last_received_timestamp;

    /**
     * The time (in milliseconds) that the last sync message was sent to the
     * client.
     */
    guac_timestamp last_sent_timestamp;

    /**
     * Reference to dlopen'd client plugin.
     */
    void* __client_plugin_handle;

    /**
     * Arbitrary reference to proxy client-specific data. Implementors of a
     * Guacamole proxy client can store any data they want here, which can then
     * be retrieved as necessary in the message handlers.
     */
    void* data;

    /**
     * Handler for server messages. If set, this function will be called
     * occasionally by the Guacamole proxy to give the client a chance to
     * handle messages from whichever server it is connected to.
     *
     * Example:
     * @code
     *     int handle_messages(guac_client* client);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->handle_messages = handle_messages;
     *     }
     * @endcode
     */
    guac_client_handle_messages* handle_messages;

    /**
     * Handler for mouse events sent by the Gaucamole web-client.
     *
     * The handler takes the integer mouse X and Y coordinates, as well as
     * a button mask containing the bitwise OR of all button values currently
     * being pressed. Those values are:
     *
     * <table>
     *     <tr><th>Button</th>          <th>Value</th></tr>
     *     <tr><td>Left</td>            <td>1</td></tr>
     *     <tr><td>Middle</td>          <td>2</td></tr>
     *     <tr><td>Right</td>           <td>4</td></tr>
     *     <tr><td>Scrollwheel Up</td>  <td>8</td></tr>
     *     <tr><td>Scrollwheel Down</td><td>16</td></tr>
     * </table>

     * Example:
     * @code
     *     int mouse_handler(guac_client* client, int x, int y, int button_mask);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->mouse_handler = mouse_handler;
     *     }
     * @endcode
     */
    guac_client_mouse_handler* mouse_handler;

    /**
     * Handler for key events sent by the Guacamole web-client.
     *
     * The handler takes the integer X11 keysym associated with the key
     * being pressed/released, and an integer representing whether the key
     * is being pressed (1) or released (0).
     *
     * Example:
     * @code
     *     int key_handler(guac_client* client, int keysym, int pressed);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->key_handler = key_handler;
     *     }
     * @endcode
     */
    guac_client_key_handler* key_handler;

    /**
     * Handler for clipboard events sent by the Guacamole web-client. This
     * handler will be called whenever the web-client sets the data of the
     * clipboard.
     *
     * This handler takes a single string which contains the text which
     * has been set in the clipboard. This text is already unescaped from
     * the Guacamole escaped version sent within the clipboard message
     * in the protocol.
     *
     * Example:
     * @code
     *     int clipboard_handler(guac_client* client, char* copied);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->clipboard_handler = clipboard_handler;
     *     }
     * @endcode
     */
    guac_client_clipboard_handler* clipboard_handler;

    /**
     * Handler for freeing data when the client is being unloaded.
     *
     * This handler will be called when the client needs to be unloaded
     * by the proxy, and any data allocated by the proxy client should be
     * freed.
     *
     * Implement this handler if you store data inside the client.
     *
     * Example:
     * @code
     *     int free_handler(guac_client* client);
     *
     *     int guac_client_init(guac_client* client, int argc, char** argv) {
     *         client->free_handler = free_handler;
     *     }
     * @endcode
     */
    guac_client_free_handler* free_handler;

    /**
     * Handler for logging informational messages. This handler will be called
     * via guac_client_log_info() when the client needs to log information.
     *
     * In general, only programs loading the client should implement this
     * handler, as those are the programs that would provide the logging
     * facilities.
     *
     * Client implementations should expect these handlers to already be
     * set.
     *
     * Example:
     * @code
     *     void log_handler(guac_client* client, const char* format, va_list args);
     *
     *     void function_of_daemon() {
     *
     *         guac_client* client = [client from guac_get_client()];
     *
     *         client->log_info_handler = log_handler;
     *
     *     }
     * @endcode
     */
    guac_client_log_handler* log_info_handler;


    /**
     * Handler for logging error messages. This handler will be called
     * via guac_client_log_error() when the client needs to log an error.
     *
     * In general, only programs loading the client should implement this
     * handler, as those are the programs that would provide the logging
     * facilities.
     *
     * Client implementations should expect these handlers to already be
     * set.
     *
     * Example:
     * @code
     *     void log_handler(guac_client* client, const char* format, va_list args);
     *
     *     void function_of_daemon() {
     *
     *         guac_client* client = [client from guac_get_client()];
     *
     *         client->log_error_handler = log_handler;
     *
     *     }
     * @endcode
     */
    guac_client_log_handler* log_error_handler;


};

/**
 * Handler which should initialize the given guac_client.
 */
typedef int guac_client_init_handler(guac_client* client, int argc, char** argv);

/**
 * Initialize and return a new guac_client. The pluggable client will be
 * chosen based on the first connect message received on the given file
 * descriptor.
 *
 * @param client_fd The file descriptor associated with the socket associated
 *                  with the connection to the web-client tunnel.
 * @param usec_timeout The maximum number of microseconds to wait for each
 *                     instruction during the initial client handshake.
 * @return A pointer to the newly initialized client.
 */
guac_client* guac_get_client(int client_fd, int usec_timeout);

/**
 * Free all resources associated with the given client.
 *
 * @param client The proxy client to free all reasources of.
 */
void guac_client_free(guac_client* client);

/**
 * Call the appropriate handler defined by the given client for the given
 * instruction. A comparison is made between the instruction opcode and the
 * initial handler lookup table defined in client-handlers.c. The intial
 * handlers will in turn call the client's handler (if defined).
 *
 * @param client The proxy client whose handlers should be called.
 * @param instruction The instruction to pass to the proxy client via the
 *                    appropriate handler.
 */
int guac_client_handle_instruction(guac_client* client, guac_instruction* instruction);

/**
 * Allocates a new buffer (invisible layer). An arbitrary index is
 * automatically assigned if no existing buffer is available for use.
 *
 * @param client The proxy client to allocate the buffer for.
 * @return The next available buffer, or a newly allocated buffer.
 */
guac_layer* guac_client_alloc_buffer(guac_client* client);

/**
 * Allocates a new layer. The layer will be given the specified index,
 * even if the layer returned was a previously used (and free'd) layer.
 *
 * @param client The proxy client to allocate the layer buffer for.
 * @param index The index of the layer to allocate.
 * @return The next available layer, or a newly allocated layer.
 */
guac_layer* guac_client_alloc_layer(guac_client* client, int index);

/**
 * Returns the given buffer to the pool of available buffers, such that it
 * can be reused by any subsequent call to guac_client_allow_buffer().
 *
 * @param client The proxy client to return the buffer to.
 * @param layer The buffer to return to the pool of available buffers.
 */
void guac_client_free_buffer(guac_client* client, guac_layer* layer);

/**
 * Logs an informational message in the log used by the given client. The
 * logger used will normally be defined by guacd (or whichever program loads
 * the proxy client) by setting the logging handlers of the client when it is
 * loaded.
 *
 * @param client The proxy client to log an informational message for.
 * @param format A printf-style format string to log.
 * @param ... Arguments to use when filling the format string for printing.
 */
void guac_client_log_info(guac_client* client, const char* format, ...);

/**
 * Logs an error message in the log used by the given client. The logger
 * used will normally be defined by guacd (or whichever program loads the
 * proxy client) by setting the logging handlers of the client when it is
 * loaded.
 *
 * @param client The proxy client to log an error for.
 * @param format A printf-style format string to log.
 * @param ... Arguments to use when filling the format string for printing.
 */
void guac_client_log_error(guac_client* client, const char* format, ...);

/**
 * Logs an informational message in the log used by the given client. The
 * logger used will normally be defined by guacd (or whichever program loads
 * the proxy client) by setting the logging handlers of the client when it is
 * loaded.
 *
 * @param client The proxy client to log an informational message for.
 * @param format A printf-style format string to log.
 * @param ap The va_list containing the arguments to be used when filling the
 *           format string for printing.
 */
void vguac_client_log_info(guac_client* client, const char* format, va_list ap);

/**
 * Logs an error message in the log used by the given client. The logger
 * used will normally be defined by guacd (or whichever program loads the
 * proxy client) by setting the logging handlers of the client when it is
 * loaded.
 *
 * @param client The proxy client to log an error for.
 * @param format A printf-style format string to log.
 * @param ap The va_list containing the arguments to be used when filling the
 *           format string for printing.
 */
void vguac_client_log_error(guac_client* client, const char* format, va_list ap);

/**
 * The default Guacamole client layer, layer 0.
 */
extern const guac_layer* GUAC_DEFAULT_LAYER;

#endif

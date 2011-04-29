
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

#ifndef _GUAC_PROTOCOL_H
#define _GUAC_PROTOCOL_H

#include <cairo/cairo.h>

#include "guacio.h"

/**
 * Provides functions and structures required for communicating using the
 * Guacamole protocol over a GUACIO connection, such as that provided by
 * guac_client objects.
 *
 * @file protocol.h
 */


/**
 * The number of milliseconds to wait for messages in any phase before
 * timing out and closing the connection with an error.
 */
#define GUAC_TIMEOUT      15000

/**
 * The number of microseconds to wait for messages in any phase before
 * timing out and closing the conncetion with an error. This is always
 * equal to GUAC_TIMEOUT * 1000.
 */
#define GUAC_USEC_TIMEOUT (GUAC_TIMEOUT*1000)

typedef int64_t guac_timestamp_t;

/**
 * Composite modes used by Guacamole draw instructions. Each
 * composite mode maps to a unique channel mask integer.
 */
typedef enum guac_composite_mode_t {

    /*
     * (NOTE - set notation encoded in UTF-8):
     *
     * A: Source where destination transparent = S ∩ D′
     * B: Source where destination opaque      = S ∩ D
     * C: Destination where source transparent = D ∩ S′
     * D: Destination where source opaque      = D ∩ S
     *
     * 0 = Active, 1 = Inactive
     */
                           /* ABCD */
    /* NOT IMPL'D:            0000 */
    GUAC_COMP_RIN   = 0x1, /* 0001 */
    GUAC_COMP_ROUT  = 0x2, /* 0010 */
    GUAC_COMP_DEST  = 0x3, /* 0011 */
    GUAC_COMP_IN    = 0x4, /* 0100 */
    /* NOT IMPL'D:            0101 */
    GUAC_COMP_ATOP  = 0x6, /* 0110 */
    /* NOT IMPL'D:            0111 */
    GUAC_COMP_OUT   = 0x8, /* 1000 */
    GUAC_COMP_RATOP = 0x9, /* 1001 */
    GUAC_COMP_XOR   = 0xA, /* 1010 */
    GUAC_COMP_ROVER = 0xB, /* 1011 */
    GUAC_COMP_SRC   = 0xC, /* 1100 */
    /* NOT IMPL'D:            1101 */
    GUAC_COMP_OVER  = 0xE, /* 1110 */
    GUAC_COMP_PLUS  = 0xF  /* 1111 */

} guac_composite_mode_t;


/**
 * Represents a single instruction within the Guacamole protocol.
 */
typedef struct guac_instruction {

    /**
     * The opcode of the instruction.
     */
    char* opcode;

    /**
     * The number of arguments passed to this instruction.
     */
    int argc;

    /**
     * Array of all arguments passed to this instruction. Strings
     * are not already unescaped.
     */
    char** argv;

} guac_instruction;


/**
 * Frees all memory allocated to the given instruction opcode
 * and arguments. The instruction structure itself will not
 * be freed.
 *
 * @param instruction The instruction to free.
 */
void guac_free_instruction_data(guac_instruction* instruction);


/**
 * Frees all memory allocated to the given instruction. This
 * includes freeing memory allocated for the structure
 * itself.
 *
 * @param instruction The instruction to free.
 */
void guac_free_instruction(guac_instruction* instruction);

/**
 * Escapes the given string as necessary to be passed within
 * a Guacamole instruction. The returned string must later be
 * released with a call to free().
 *
 * @param str The string to escape.
 * @return A new escaped string, which must be freed with free().
 */
char* guac_escape_string(const char* str);

/**
 * Unescapes the given string in-place, as an unescaped string
 * is always the same length or shorter than the original.
 *
 * @param str The string to unescape.
 * @return A pointer to the original string, which is now unescaped.
 */
char* guac_unescape_string_inplace(char* str);

/**
 * Sends an args instruction over the given GUACIO connection. Each
 * argument name will be automatically escaped for transmission.
 *
 * @param io The GUACIO connection to use.
 * @param args The NULL-terminated array of argument names (strings).
 * @return Zero on success, non-zero on error.
 */
int guac_send_args(GUACIO* io, const char** name);

/**
 * Sends a name instruction over the given GUACIO connection. The
 * name given will be automatically escaped for transmission.
 *
 * @param io The GUACIO connection to use.
 * @param name The name to send within the name instruction.
 * @return Zero on success, non-zero on error.
 */
int guac_send_name(GUACIO* io, const char* name);

/**
 * Sends a sync instruction over the given GUACIO connection. The
 * current time in milliseconds should be passed in as the timestamp.
 *
 * @param io The GUACIO connection to use.
 * @param timestamp The current timestamp (in milliseconds).
 * @return Zero on success, non-zero on error.
 */
int guac_send_sync(GUACIO* io, guac_timestamp_t timestamp);

/**
 * Sends an error instruction over the given GUACIO connection. The
 * error description given will be automatically escaped for
 * transmission.
 *
 * @param io The GUACIO connection to use.
 * @param error The description associated with the error.
 * @return Zero on success, non-zero on error.
 */
int guac_send_error(GUACIO* io, const char* error);

/**
 * Sends a clipboard instruction over the given GUACIO connection. The
 * clipboard data given will be automatically escaped for transmission.
 *
 * @param io The GUACIO connection to use.
 * @param data The clipboard data to send.
 * @return Zero on success, non-zero on error.
 */
int guac_send_clipboard(GUACIO* io, const char* data);

/**
 * Sends a size instruction over the given GUACIO connection.
 *
 * @param io The GUACIO connection to use.
 * @param w The width of the display.
 * @param h The height of the display.
 * @return Zero on success, non-zero on error.
 */
int guac_send_size(GUACIO* io, int w, int h);

/**
 * Sends a copy instruction over the given GUACIO connection.
 *
 * @param io The GUACIO connection to use.
 * @param srcl The index of the source layer.
 * @param srcx The X coordinate of the source rectangle.
 * @param srcy The Y coordinate of the source rectangle.
 * @param w The width of the source rectangle.
 * @param h The height of the source rectangle.
 * @param mode The composite mode to use.
 * @param dstl The index of the destination layer.
 * @param dstx The X coordinate of the destination, where the source rectangle
 *             should be copied.
 * @param dsty The Y coordinate of the destination, where the source rectangle
 *             should be copied.
 * @return Zero on success, non-zero on error.
 */
int guac_send_copy(GUACIO* io, 
        int srcl, int srcx, int srcy, int w, int h,
        guac_composite_mode_t mode, int dstl, int dstx, int dsty);

/**
 * Sends a png instruction over the given GUACIO connection. The PNG image data
 * given will be automatically base64-encoded for transmission.
 *
 * @param io The GUACIO connection to use.
 * @param mode The composite mode to use.
 * @param layer The index of the destination layer.
 * @param x The destination X coordinate.
 * @param y The destination Y coordinate.
 * @param surface A cairo surface containing the image data to send.
 * @return Zero on success, non-zero on error.
 */
int guac_send_png(GUACIO* io, guac_composite_mode_t mode,
        int layer, int x, int y, cairo_surface_t* surface);

/**
 * Sends a cursor instruction over the given GUACIO connection. The PNG image
 * data given will be automatically base64-encoded for transmission.
 *
 * @param io The GUACIO connection to use.
 * @param x The X coordinate of the cursor hotspot.
 * @param y The Y coordinate of the cursor hotspot.
 * @param surface A cairo surface containing the image data to send.
 * @return Zero on success, non-zero on error.
 */
int guac_send_cursor(GUACIO* io, int x, int y, cairo_surface_t* surface);

/**
 * Returns whether new instruction data is available on the given GUACIO
 * connection for parsing.
 *
 * @param io The GUACIO connection to use.
 * @return A positive value if data is available, negative on error, or
 *         zero if no data is currently available.
 */
int guac_instructions_waiting(GUACIO* io);

/**
 * Reads a single instruction from the given GUACIO connection.
 *
 * @param io The GUACIO connection to use.
 * @param parsed_instruction A pointer to a guac_instruction structure which
 *                           will be populated with data read from the given
 *                           GUACIO connection.
 * @return A positive value if data was successfully read, negative on
 *         error, or zero if the instruction could not be read completely
 *         because GUAC_TIMEOUT elapsed, in which case subsequent calls to
 *         guac_read_instruction() will return the parsed instruction once
 *         enough data is available.
 */
int guac_read_instruction(GUACIO* io, guac_instruction* parsed_instruction);

guac_timestamp_t guac_current_timestamp();
void guac_sleep(int millis);

#endif


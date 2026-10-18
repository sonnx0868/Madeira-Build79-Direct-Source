/* Rename native C linkage without rewriting Wine's request/structure tokens.
 * This is the same symbol map used by build.sh's archive objcopy sweep.
 * Force-include BEFORE Wine headers when refreshing individual server objects. */
#pragma redefine_extname alloc_user_handle _ws_alloc_user_handle
#pragma redefine_extname free_user_handle _ws_free_user_handle
#pragma redefine_extname get_virtual_screen_rect _ws_get_virtual_screen_rect
#pragma redefine_extname destroy_thread_windows _ws_destroy_thread_windows
#pragma redefine_extname get_window_thread _ws_get_window_thread
#pragma redefine_extname is_desktop_class _ws_is_desktop_class
#pragma redefine_extname is_message_class _ws_is_message_class
#pragma redefine_extname is_window_visible _ws_is_window_visible
#pragma redefine_extname mirror_region _ws_mirror_region
#pragma redefine_extname send_notify_message _ws_send_notify_message
#pragma redefine_extname shared_session _ws_shared_session
#pragma redefine_extname user_shared_data _ws_user_shared_data

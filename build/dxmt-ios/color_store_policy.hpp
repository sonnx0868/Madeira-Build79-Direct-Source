/* SPDX-License-Identifier: MIT
 * A full clear makes the immediately preceding store of the same base view
 * unnecessary. Relation/dependency ordering remains the encoder scheduler's
 * responsibility; this helper never changes its read/write sets or barriers.
 */
#pragma once
template<typename Render, typename Clear, typename Color>
static inline bool madeiraCanDiscardClearedColor(const Render *render, const Clear *clear, const Color &color) {
    return !clear->clear_dsv && clear->attachment &&
        render->render_target_width && render->render_target_height && render->render_target_array_length &&
        clear->array_length == render->render_target_array_length &&
        clear->width == render->render_target_width && clear->height == render->render_target_height &&
        color.attachment == clear->attachment && !color.level && !color.slice && !color.depth_plane &&
        !color.resolve_attachment && color.store_action == WMTStoreActionStore;
}

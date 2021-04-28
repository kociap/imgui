#pragma once

#include <imconfig.h>

#include <anton/slice.hpp>

namespace ImGui::drag_drop {
    // is_active
    // Checks whether a drag and drop operation is in process.
    //
    // Returns:
    // true when a drag and drop operation is in process, false otherwise.
    //
    [[nodiscard]] bool is_active();

    // clear
    // Clears the drag and drop context essentially cancelling the operation.
    //
    void clear();

    // set_payload
    // Sets the drag and drop payload. data is copied and stored internally.
    //
    // Parameters:
    // payload_id - ID identifying the payload. The exact same ID must be 
    //              passed to accept_payload() for the payload to be delivered.
    //       data - payload data to be stored.
    //
    void set_payload(u64 payload_id, anton::Slice<u8 const> data);

    // get_payload
    // Obtains the stored payload.
    //
    // Returns:
    // A slice containing the data as u8.
    //
    [[nodiscard]] anton::Slice<u8> get_payload();

    // accept_payload
    // Checks whether there is a payload with matching payload_id awaiting delivery.
    // If true, marks the payload as delivered and ready to be cleaned up at the end of the frame.
    // Ends the drag and drop operation.
    //
    // To retrieve the payload data call get_payload().
    //
    // Parameters:
    // payload_id - ID identifying the payload.
    //
    // Returns:
    // true when there is a payload with matching payload_id ready to be delivered.
    // false otherwise.
    //
    [[nodiscard]] bool accept_payload(u64 payload_id);

    // begin_source
    // Mark the last widget as a drag and drop source.
    // The return value indicates whether a drag and drop operation has been started.
    // You should set the payload via set_payload() when the return value is true.
    // Otherwise the operation will be cancelled.
    //
    // end_source() must be called after this function only when the return value is true.
    //
    // Returns:
    // true when a drag and drop operation has been started, false otherwise.
    //
    [[nodiscard]] bool begin_source();

    // end_source
    // End drag and drop source related processing.
    // Must only be called after begin_source() when it returned true.
    //
    void end_source();

    // begin_target
    // Mark the last widget as a drag and drop target.
    // The return value indicates whether a drag and drop target can accept the payload.
    // You should accept the payload via accept_payload() when the return value is true.
    //
    // end_target() must be called after this function only when the return value is true.
    //
    // Returns:
    // true when, false otherwise.
    //
    [[nodiscard]] bool begin_target();

    // end_target
    // End drag and drop target related processing.
    // Must only be called after begin_target() when it returned true.
    //
    void end_target();
}
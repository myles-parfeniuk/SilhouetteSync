#include "RequestHandler.hpp"

RequestHandler::RequestHandler(Device& d, PacketTransceiver& packet_stream)
    : d(d)
    , packet_stream(packet_stream)
{
}

void RequestHandler::handle_sample(payload_t* transmit_buffer)
{
    packet_stream.send_sample_packet(transmit_buffer);
}

void RequestHandler::handle_tare(payload_t* transmit_buffer)
{
    d.imu.state.set(IMUState::tare);
    packet_stream.send_sample_packet(transmit_buffer);
}

void RequestHandler::handle_calibration(payload_t* transmit_buffer)
{
    Responses response = Responses::server_failure;
    uint16_t response_attempts = 0;

    d.imu.state.set(IMUState::calibrate);

    // service request and send busy packets until complete
    do
    {
        if (packet_stream.receive_packet(transmit_buffer))
            packet_stream.send_busy_packet(transmit_buffer);

    } while (d.imu.state.get() == IMUState::calibrate);

    // send until success/failure until affirmative response
    do
    {
        if (packet_stream.receive_packet(transmit_buffer))
        {
            response = static_cast<Responses>(transmit_buffer->response);

            if (response != Responses::client_affirmative)
            {

                if (d.imu.calibration_status.get())
                    packet_stream.send_success_packet(transmit_buffer);
                else
                    packet_stream.send_failure_packet(transmit_buffer);
            }

            response_attempts++;

            if (response_attempts >= MAX_RESPONSE_ATTEMPTS)
                break;
        }

    } while (response != Responses::client_affirmative);
}
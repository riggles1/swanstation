#include "playstation_mouse.h"
#include "common/state_wrapper.h"
#include "gpu.h"
#include "host_display.h"
#include "host_interface.h"
#include "system.h"
#include <array>

PlayStationMouse::PlayStationMouse()
{
  m_last_host_position_x = g_host_interface->GetDisplay()->GetMousePositionX();
  m_last_host_position_y = g_host_interface->GetDisplay()->GetMousePositionY();
}

PlayStationMouseBall::~PlayStationMouseBall() = default;

ControllerType PlayStationMouseBall::GetType() const
{
  return ControllerType::PlayStationMouseBall;
}

void PlayStationMouseBall::Reset()
{
  m_transfer_state = TransferState::Idle;
}

bool PlayStationMouseBall::DoState(StateWrapper& sw, bool apply_input_state)
{
  if (!Controller::DoState(sw, apply_input_state))
    return false;

  u16 button_state = m_button_state;
  u8 delta_x = m_delta_x;
  u8 delta_y = m_delta_y;
  sw.Do(&button_state);
  sw.Do(&delta_x);
  sw.Do(&delta_y);
  if (apply_input_state)
  {
    m_button_state = button_state;
    m_delta_x = delta_x;
    m_delta_y = delta_y;
  }

  sw.Do(&m_transfer_state);
  return true;
}

void PlayStationMouseBall::SetButtonState(Button button, bool pressed)
{
  static constexpr std::array<u8, static_cast<size_t>(Button::Count)> indices = {{11, 10}};
  if (pressed)
    m_button_state &= ~(u16(1) << indices[static_cast<u8>(button)]);
  else
    m_button_state |= u16(1) << indices[static_cast<u8>(button)];
}

void PlayStationMouseBall::SetButtonState(s32 button_code, bool pressed)
{
  if (button_code < 0 || button_code >= static_cast<s32>(Button::Count))
    return;

  SetButtonState(static_cast<Button>(button_code), pressed);
}

void PlayStationMouseBall::ResetTransferState()
{
  m_transfer_state = TransferState::Idle;
}

bool PlayStationMouseBall::Transfer(const u8 data_in, u8* data_out)
{
  static constexpr u16 ID = 0x5A12;

  switch (m_transfer_state)
  {
    case TransferState::Idle:
    {
      *data_out = 0xFF;

      if (data_in == 0x01)
      {
        m_transfer_state = TransferState::Ready;
        return true;
      }
      break;
    }

    case TransferState::Ready:
    {
      if (data_in == 0x42)
      {
        *data_out = Truncate8(ID);
        m_transfer_state = TransferState::IDMSB;
        return true;
      }

      *data_out = 0xFF;
      break;
    }

    case TransferState::IDMSB:
    {
      *data_out = Truncate8(ID >> 8);
      m_transfer_state = TransferState::ButtonsLSB;
      return true;
    }

    case TransferState::ButtonsLSB:
    {
      *data_out = Truncate8(m_button_state);
      m_transfer_state = TransferState::ButtonsMSB;
      return true;
    }

    case TransferState::ButtonsMSB:
    {
      *data_out = Truncate8(m_button_state >> 8);
      m_transfer_state = TransferState::DeltaX;
      return true;
    }

    case TransferState::DeltaX:
    {
      UpdatePosition();
      *data_out = static_cast<u8>(m_delta_x);
      m_transfer_state = TransferState::DeltaY;
      return true;
    }

    case TransferState::DeltaY:
    {
      *data_out = static_cast<u8>(m_delta_y);
      m_transfer_state = TransferState::Idle;
      break;
    }

    default:
      break;
  }

  return false;
}

void PlayStationMousBalle::UpdatePosition()
{
  // get screen coordinates
  const HostDisplay* display = g_host_interface->GetDisplay();
  const s32 mouse_x = display->GetMousePositionX();
  const s32 mouse_y = display->GetMousePositionY();
  const s32 delta_x = mouse_x - m_last_host_position_x;
  const s32 delta_y = mouse_y - m_last_host_position_y;
  m_last_host_position_x = mouse_x;
  m_last_host_position_y = mouse_y;

  m_delta_x = static_cast<s8>(std::clamp<s32>(delta_x, std::numeric_limits<s8>::min(), std::numeric_limits<s8>::max()));
  m_delta_y = static_cast<s8>(std::clamp<s32>(delta_y, std::numeric_limits<s8>::min(), std::numeric_limits<s8>::max()));
}

std::unique_ptr<PlayStationMouseBall> PlayStationMouseBall::Create()
{
  return std::make_unique<PlayStationMouseBall>();
}

u32 PlayStationMouseBall::StaticGetVibrationMotorCount()
{
  return 0;
}

#pragma once
#include "controller.h"
#include <memory>
#include <optional>
#include <string_view>

class PlayStationMouseBall final : public Controller
{
public:
  enum class Button : u8
  {
    Left = 0,
    Right = 1,
    Count
  };

  PlayStationMouseBall();
  ~PlayStationMouseBall() override;

  static std::unique_ptr<PlayStationMouseBall> Create();
  static u32 StaticGetVibrationMotorCount();

  ControllerType GetType() const override;

  void Reset() override;
  bool DoState(StateWrapper& sw, bool apply_input_state) override;

  void SetButtonState(s32 button_code, bool pressed) override;

  void ResetTransferState() override;
  bool Transfer(const u8 data_in, u8* data_out) override;

  void SetButtonState(Button button, bool pressed);

private:
  void UpdatePosition();

  enum class TransferState : u8
  {
    Idle,
    Ready,
    IDMSB,
    ButtonsLSB,
    ButtonsMSB,
    DeltaX,
    DeltaY
  };

  s32 m_last_host_position_x = 0;
  s32 m_last_host_position_y = 0;

  // buttons are active low
  u16 m_button_state = UINT16_C(0xFFFF);
  s8 m_delta_x = 0;
  s8 m_delta_y = 0;

  TransferState m_transfer_state = TransferState::Idle;
};

#pragma once
#include "core/host_interface.h"
#include "core/system.h"
#include <libretro.h>
#include <limits>
#include <optional>
#include <memory>
#include <vector>
#include <string>

namespace GameSettings
{
struct Entry;
}

class LibretroHostInterface final : public HostInterface
{
public:

  struct DiskControlInfo
  {
    bool has_sub_images;
    u32 initial_image_index;
    u32 image_index;
    u32 image_count;
    std::string sub_images_parent_path;
    std::vector<std::string> image_paths;
    std::vector<std::string> image_labels;
  };

  LibretroHostInterface();
  ~LibretroHostInterface() override;

  ALWAYS_INLINE u32 GetResolutionScale() const { return (g_settings.gpu_downsample_mode == GPUDownsampleMode::Box) ? 1u : g_settings.gpu_resolution_scale; }

  bool Initialize() override;
  void Shutdown() override;

  void ReportError(const char* message) override;
  void ReportMessage(const char* message) override;
  bool ConfirmMessage(const char* message) override;
  void AddOSDMessage(std::string message, float duration = 2.0f) override;
  void DisplayLoadingScreen(const char* message, int progress_min = -1, int progress_max = -1,
                            int progress_value = -1) override;

  void GetGameInfo(const char* path, CDImage* image, std::string* code, std::string* title) override;
  std::string GetSharedMemoryCardPath(u32 slot) const override;
  std::string GetGameMemoryCardPath(const char* game_code, u32 slot) const override;
  std::string GetShaderCacheBasePath() const override;
  std::string GetStringSettingValue(const char* section, const char* key, const char* default_value = "") override;
  std::string GetBIOSDirectory() override;

  bool UpdateSystemAVInfo(bool use_resolution_scale);

  bool UpdateCoreOptionsDisplay(bool controller);

  // Called by frontend
  void retro_set_environment();
  void retro_get_system_av_info(struct retro_system_av_info* info);
  bool retro_load_game(const struct retro_game_info* game);
  void retro_set_controller_port_device(u32 port, u32 device);
  void retro_run_frame();
  unsigned retro_get_region();
  size_t retro_serialize_size();
  bool retro_serialize(void* data, size_t size);
  bool retro_unserialize(const void* data, size_t size);
  void* retro_get_memory_data(unsigned id);
  size_t retro_get_memory_size(unsigned id);
  void retro_cheat_reset();
  void retro_cheat_set(unsigned index, bool enabled, const char* code);

protected:
  void AcquireHostDisplay() override;
  void ReleaseHostDisplay() override;
  std::unique_ptr<AudioStream> CreateAudioStream() override;
  void CheckForSettingsChanges(const Settings& old_settings) override;
  void OnRunningGameChanged(const std::string& path, CDImage* image, const std::string& game_code,
                            const std::string& game_title) override;
  void OnControllerTypeChanged(u32 slot) override;

  void SetMouseMode(bool relative, bool hide_cursor) override;

private:
  bool HasCoreVariablesChanged();
  void InitInterfaces();
  void InitLogging();
  void InitDiskControlInterface();
  void InitRumbleInterface();

  void LoadSettings();
  void UpdateSettings();
  void UpdateControllers();
  void UpdateControllersDigitalController(u32 index);
  void UpdateControllersAnalogController(u32 index);
  void UpdateControllersAnalogJoystick(u32 index);
  void UpdateControllersNeGcon(u32 index);
  void UpdateControllersNeGconRumble(u32 index);
  void UpdateControllersNamcoGunCon(u32 index);
  void UpdateControllersPlayStationMouse(u32 index);
  void UpdateControllersPlayStationMouseBall(u32 index);
  void GetSystemAVInfo(struct retro_system_av_info* info, bool use_resolution_scale);
  void UpdateGeometry();
  void UpdateLogging();

  bool UpdateGameSettings();
  void ApplyGameSettings();

  static bool RETRO_CALLCONV UpdateCoreOptionsDisplayCallback();

  // Hardware renderer setup.
  bool RequestHardwareRendererContext();
  void SwitchToHardwareRenderer();
  void SwitchToSoftwareRenderer();

  static void HardwareRendererContextReset();
  static void HardwareRendererContextDestroy();

  // Disk control callbacks
  static bool RETRO_CALLCONV DiskControlSetEjectState(bool ejected);
  static bool RETRO_CALLCONV DiskControlGetEjectState();
  static unsigned RETRO_CALLCONV DiskControlGetImageIndex();
  static bool RETRO_CALLCONV DiskControlSetImageIndex(unsigned index);
  static unsigned RETRO_CALLCONV DiskControlGetNumImages();
  static bool RETRO_CALLCONV DiskControlReplaceImageIndex(unsigned index, const retro_game_info* info);
  static bool RETRO_CALLCONV DiskControlAddImageIndex();
  static bool RETRO_CALLCONV DiskControlSetInitialImage(unsigned index, const char* path);
  static bool RETRO_CALLCONV DiskControlGetImagePath(unsigned index, char* path, size_t len);
  static bool RETRO_CALLCONV DiskControlGetImageLabel(unsigned index, char* label, size_t len);

  std::unique_ptr<GameSettings::Entry> m_game_settings;
  float m_last_aspect_ratio = 4.0f / 3.0f;

  std::array<u32, NUM_CONTROLLER_AND_CARD_PORTS> retropad_device = {RETRO_DEVICE_JOYPAD};

  bool controller_dirty = false;

  retro_hw_render_callback m_hw_render_callback = {};
  std::unique_ptr<HostDisplay> m_hw_render_display;
  bool m_hw_render_callback_valid = false;
  bool m_using_hardware_renderer = false;

  retro_rumble_interface m_rumble_interface = {};
  bool m_rumble_interface_valid = false;
  bool m_supports_input_bitmasks = false;

  DiskControlInfo m_disk_control_info = {};
};

extern LibretroHostInterface g_libretro_host_interface;

// libretro callbacks
extern retro_environment_t g_retro_environment_callback;
extern retro_video_refresh_t g_retro_video_refresh_callback;
extern retro_audio_sample_t g_retro_audio_sample_callback;
extern retro_audio_sample_batch_t g_retro_audio_sample_batch_callback;
extern retro_input_poll_t g_retro_input_poll_callback;
extern retro_input_state_t g_retro_input_state_callback;

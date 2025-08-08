#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include <libdragon.h>
#include "libcart/cart.h"

#include "libh8300h/devices/bma150.h"
#include "libh8300h/devices/buttons.h"
#include "libh8300h/devices/eeprom.h"
#include "libh8300h/devices/lcd.h"
#include "libh8300h/devices/led.h"
#include "libh8300h/system.h"

#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 64

typedef struct
{
  h8_system_t system;
  h8_lcd_t *lcd;
  h8_device_t *bma150;
  h8_buttons_t *buttons;
  h8_device_t *eeprom;
  h8_device_t *led;
  unsigned short *video_buffer;
  surface_t video_frame;
  int state;
  int frames;
  sprite_t *icon;
} pfu_ctx_ctx_t;

static pfu_ctx_ctx_t ctx;

static int h8u_load_file(void *dst, unsigned size, const char *path)
{
  FILE *file;
  char fullpath[256];

  snprintf(fullpath, sizeof(fullpath), "rom:/roms/%s", path);
  file = fopen(fullpath, "r");
  if (file)
  {
    unsigned file_size;

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);

    if (file_size > size)
    {
      fclose(file);
      printf("File %s is too large (%u bytes) for destination (%u bytes)\n",
             path, file_size, size);
      return 0;
    }
    else
    {
      rewind(file);
      fread(dst, sizeof(char), size, file);
      fclose(file);
      printf("Loaded %s (%u bytes)\n", path, file_size);

      return 1;
    }
  }
  printf("Failed to open file %s: %s\n", fullpath, strerror(errno));
  exit(0);

  return 0;
}

static h8_device_t *h8lr_find_device(unsigned type)
{
  unsigned i;

  for (i = 0; i < ctx.system.device_count; i++)
    if (ctx.system.devices[i].type == type)
      return &ctx.system.devices[i];

  return NULL;
}

static void h8u_emu_input(void)
{
  joypad_buttons_t buttons;
  h8_u16 shake;

  joypad_poll();
  buttons = joypad_get_buttons(JOYPAD_PORT_1);
  shake = buttons.z ? 0x3ff : 0x000;

  ctx.buttons->buttons[H8_BUTTON_MAIN] = buttons.a;
  ctx.buttons->buttons[H8_BUTTON_LEFT] = buttons.d_left;
  ctx.buttons->buttons[H8_BUTTON_RIGHT] = buttons.d_right;
  h8_bma150_set_axis(ctx.bma150, shake, 0, shake);
}

static h8_u16 h8u_colors[4] =
{
  0xB5AD,
  0x8421,
  0x6319,
  0x18C7
};

static const rdpq_blitparms_t h8u_1_1_480p_params = {
    .scale_x = 6.0f,
    .scale_y = 6.0f };
static void h8u_video_render(void)
{
  surface_t *disp = display_get();
  
  rdpq_attach_clear(disp, NULL);
  rdpq_set_mode_standard();
  rdpq_tex_blit(&ctx.video_frame,
                32,
                48,
                &h8u_1_1_480p_params);
  rdpq_detach_show();
}

int main(void)
{
  h8_device_t *device;

  memset(&ctx, 0, sizeof(ctx));

  /* Initialize controller */
  joypad_init();

  console_init();
  console_set_render_mode(RENDER_AUTOMATIC);
  console_clear();
  printf("Stepdad tests on N64...\n");
  h8_test();

  /* Initialize assets */
  dfs_init(DFS_DEFAULT_LOCATION);

  /* Initialize fonts
  rdpq_font_t *font1 = rdpq_font_load("rom:/Tuffy_Bold.font64");
  rdpq_text_register_font(1, font1);
  rdpq_font_style(font1, 0, &(rdpq_fontstyle_t){
	                .color = RGBA32(255, 255, 255, 255),
	                .outline_color = RGBA32(0, 0, 0, 255)});
  rdpq_font_t *font2 = rdpq_font_load("rom:/Tuffy_Bold.font64");
  rdpq_text_register_font(2, font2);
  rdpq_font_style(font2, 0, &(rdpq_fontstyle_t){
	                .color = RGBA32(0, 0, 0, 127),
	                .outline_color = RGBA32(0, 0, 0, 127)});
  rdpq_font_t *font3 = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
  rdpq_text_register_font(3, font3);

  ctx.icon = sprite_load("rom:/icon.sprite");
  */

  /* Initialize audio
  audio_init(PF_SOUND_FREQUENCY, 4);
  */

  /* Initialize emulator */
  h8_system_init(&ctx.system, H8_SYSTEM_NTR_032);

  device = h8lr_find_device(H8_DEVICE_LCD);
  if (device)
    ctx.lcd = (h8_lcd_t*)device->device;

  device = h8lr_find_device(H8_DEVICE_3BUTTON);
  if (!device)
    device = h8lr_find_device(H8_DEVICE_1BUTTON);
  if (device)
    ctx.buttons = (h8_buttons_t*)device->device;

  device = h8lr_find_device(H8_DEVICE_EEPROM_64K);
  if (!device)
    device = h8lr_find_device(H8_DEVICE_EEPROM_8K);
  if (device)
    ctx.eeprom = device;

  device = h8lr_find_device(H8_DEVICE_BMA150);
  if (device)
    ctx.bma150 = device;

  /* Load files */
  h8u_load_file(ctx.system.vmem.raw, 48 * 1024, "ntr032.bin");
  h8u_load_file(ctx.eeprom->data, 64 * 1024, "ntr032-eep.bin");
  h8_init(&ctx.system);


  /* If loaded as plugin, jump to loaded ROM, otherwise load ROM menu
  if (pfu_plugin_read_rom())
    pfu_ctx_switch();
  else
    pfu_menu_switch_roms();
  */

  console_close();  

  /* Initialize video */
  display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
  rdpq_init();
  ctx.video_buffer = (unsigned short*)malloc_uncached_aligned(64, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
  ctx.video_frame = surface_make_linear(ctx.video_buffer, FMT_RGBA16, SCREEN_WIDTH, SCREEN_HEIGHT);

  while (64)
  {
    int x, y;
    unsigned i;

    h8u_emu_input();

    for (i = 0; i < 8000; i++)
      h8_step(&ctx.system);

    for (y = ctx.lcd->start_line; y < 64; y++)
    {
      for (x = 0; x < 128 - ctx.lcd->display_offset; x++)
      {
        int pg = y / 8;
        int b = y % 8;
        int col = x;
        int byte = ((ctx.lcd->vram[pg*0x100 + col*2] << 8) | ctx.lcd->vram[pg*0x100 + col*2+1]) >> b;
        int color = ((byte & 0x100) >> 7) | (byte & 1);
        
        ctx.video_buffer[y * 96 + x] = h8u_colors[color];
      }
    }
    h8u_video_render();

    ctx.frames++;
  }
}

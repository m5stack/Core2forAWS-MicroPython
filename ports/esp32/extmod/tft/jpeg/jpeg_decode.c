#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "py/stream.h"

#include "extmod/vfs.h"
#include "extmod/vfs_lfs.h"

#include "esp32/rom/tjpgd.h"
#include "esp_heap_caps.h"

#include "../tfthost.h"

//Size of the work space for the jpeg decoder.
#define WORKSZ 3800
#define LINEBUF_SIZE 512

typedef struct {
  tft_host_t* tft_host;
  mp_obj_t *file_read;
  const mp_stream_p_t *stream_p;
  int x;				// image top left point X position
  int y;				// image top left point Y position
  uint8_t* membuff;		// memory buffer containing the image
  uint32_t bufsize;		// size of the memory buffer
  uint8_t* bufptr;			// memory buffer current position
  uint8_t* linbuf[2];		// memory buffer used for display output
  uint8_t linbuf_idx;
} JpegDev;

static UINT tjd_buf_input(JDEC* jd, BYTE* buff, UINT nd) {
  return 0;
}

static UINT tjd_file_input(JDEC* decoder, BYTE* buff, UINT nd) {
  JpegDev* jd = (JpegDev *)decoder->device;
  if (buff) {
    int error = 0;
    return jd->stream_p->read(jd->file_read, buff, nd, &error);
  } else {
    struct mp_stream_seek_t seek_s;
    int error = 0;
    seek_s.offset = nd;
    seek_s.whence = SEEK_CUR;
    if (jd->stream_p->ioctl(jd->file_read, MP_STREAM_SEEK, (mp_uint_t)(uintptr_t)&seek_s, &error) == MP_STREAM_ERROR) {
      return 0;
    } else {
      return nd;
    }
  }
}

static UINT tjd_output (JDEC* decoder, void* bitmap,	JRECT* rect) {
  JpegDev* jd = (JpegDev*)decoder->device;
  uint8_t *dest = (uint8_t *)jd->linbuf[jd->linbuf_idx];
  uint8_t *src = (uint8_t*)bitmap;
  int x;
	int y;
	int dleft, dtop, dright, dbottom;

	int left = rect->left + jd->x;
	int top = rect->top + jd->y;
	int right = rect->right + jd->x;
	int bottom = rect->bottom + jd->y;

	if ((left > jd->tft_host->text_cursor.x2) || (top > jd->tft_host->text_cursor.y2)) return 1;	// out of screen area, return
	if ((right < jd->tft_host->text_cursor.x1) || (bottom < jd->tft_host->text_cursor.y1)) return 1;// out of screen area, return

	if (left < jd->tft_host->text_cursor.x1) dleft = jd->tft_host->text_cursor.x1;
	else dleft = left;
	if (top < jd->tft_host->text_cursor.y1) dtop = jd->tft_host->text_cursor.y1;
	else dtop = top;
	if (right > jd->tft_host->text_cursor.x2) dright = jd->tft_host->text_cursor.x2;
	else dright = right;
	if (bottom > jd->tft_host->text_cursor.y2) dbottom = jd->tft_host->text_cursor.y2;
	else dbottom = bottom;

	if ((dleft > jd->tft_host->text_cursor.x2) || (dtop > jd->tft_host->text_cursor.y2)) return 1;		// out of screen area, return
	if ((dright < jd->tft_host->text_cursor.x1) || (dbottom < jd->tft_host->text_cursor.y1)) return 1;	// out of screen area, return

	if ((dleft > jd->tft_host->_width) || (dtop > jd->tft_host->_hight)) return 1;		// out of screen area, return
	if ((dright < 0) || (dbottom < 0)) return 1;	// out of screen area, return


  for (y = top; y <= bottom; y++) {
    for (x = left; x <= right; x++) {
      if ((x >= dleft) && (y >= dtop) && (x <= dright) && (y <= dbottom)) {
        *dest = (*src & 0xf8) | (*(src + 1) >> 5);
        *(dest + 1) = ((*(src + 1) << 3) & 0xe0) | ((*(src + 2) & 0xf8) >> 3);
        dest += 2;
      }
      src += 3;
    }
  }

  uint32_t len = ((dright - dleft + 1) * (dbottom - dtop + 1));

  jd->tft_host->deselect(jd->tft_host);
  jd->tft_host->pushColorBuffer(jd->tft_host, dleft, dtop, dright, dbottom, jd->linbuf[jd->linbuf_idx], 2 * len, false);
  jd->linbuf_idx = 1 - jd->linbuf_idx;

  return 1;
}

void jpeg_draw_image(tft_host_t* host, int x, int y, uint8_t scale, const char *fname, uint8_t *buf, int size) {
  JpegDev jd;
  JDEC decoder;
  JRESULT rc;
  char *work = NULL;
  
  if (fname == NULL && buf == NULL) {
    return ;
  }

  jd.x = x;
  jd.y = y;
  jd.tft_host = host;
  jd.file_read = NULL;
  jd.stream_p = NULL;

  // Try to open file first, need error behind malloc
  if (fname != NULL) {
    mp_arg_val_t args[2];
    args[0].u_obj = mp_obj_new_str_of_type(&mp_type_str, (const byte *)fname, strlen(fname));
    args[1].u_rom_obj = MP_ROM_QSTR(MP_QSTR_rb);
    jd.file_read = mp_call_function_n_kw(MP_OBJ_FROM_PTR(&mp_vfs_open_obj), 2, 0, (mp_obj_t)args);
    jd.stream_p = mp_get_stream(jd.file_read);
  }

  work = calloc(WORKSZ, 1);
  if (work == NULL) {
    m_malloc_fail(WORKSZ);
    return ;
  }

  jd.linbuf[0] = heap_caps_malloc(LINEBUF_SIZE, MALLOC_CAP_DEFAULT | MALLOC_CAP_DMA);
  jd.linbuf[1] = heap_caps_malloc(LINEBUF_SIZE, MALLOC_CAP_DEFAULT | MALLOC_CAP_DMA);
  jd.linbuf_idx = 0;

  // used name first
  if (fname != NULL) {
    rc = jd_prepare(&decoder, tjd_file_input, (void *)work, WORKSZ, &jd);
  } else {
    jd.bufptr = buf;
    jd.bufsize = size;
    rc = jd_prepare(&decoder, tjd_buf_input, (void *)work, WORKSZ, &jd);
  }

  if (rc != JDR_OK) {
    goto exit;
  }

  host->select(host);
  rc = jd_decomp(&decoder, tjd_output, scale);
  host->deselect(host);
  if (rc != JDR_OK) {
    goto exit;
  }

exit:
	if (work) free(work);  // free work buffer
	if (jd.linbuf[0]) free(jd.linbuf[0]);
	if (jd.linbuf[1]) free(jd.linbuf[1]);
  if (jd.stream_p != NULL) {
    int error = 0;
    jd.stream_p->ioctl(jd.file_read, MP_STREAM_CLOSE, 0, &error);
  }
}

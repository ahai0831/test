#include <cstdio>
#include <cstdlib>

#include <windows.h>

#ifdef _MSC_VER
#define ssize_t SSIZE_T
#pragma warning(disable : 4996)
#endif
#include <vlc/vlc.h>

int vlc_local_test(const char *local_file_path) {
  libvlc_instance_t *inst;
  libvlc_media_player_t *mp;
  libvlc_media_t *m;

  ///  On Microsoft Windows Vista/2008, the process error mode
  /// SEM_FAILCRITICALERRORS flag <b>must< / b> be set before using LibVLC.
  /// On later versions, that is optional and unnecessary.
  OSVERSIONINFO osvi{sizeof(OSVERSIONINFO)};
  GetVersionEx(&osvi);
  if (6 > osvi.dwMajorVersion ||
      6 == osvi.dwMajorVersion && 0 == osvi.dwMinorVersion) {
    SetErrorMode(SEM_FAILCRITICALERRORS);
  }
  inst = libvlc_new(0, NULL);                        /* Load the VLC engine */
  m = libvlc_media_new_path(inst, local_file_path);  // must be english path
  mp = libvlc_media_player_new_from_media(
      m);                  /* Create a media player playing environement */
  libvlc_media_release(m); /* No need to keep the media now */

  libvlc_media_player_play(mp); /* play the media_player */
  Sleep(15 * 1000);             /* Let it play a bit */

  libvlc_media_player_stop(mp);    /* Stop playing */
  libvlc_media_player_release(mp); /* Free the media_player */
  libvlc_release(inst);
  return 0;
}
int vlc_url_test(const char *url) {
  libvlc_instance_t *inst;
  libvlc_media_player_t *mp;
  libvlc_media_t *m;

  ///  On Microsoft Windows Vista/2008, the process error mode
  /// SEM_FAILCRITICALERRORS flag <b>must< / b> be set before using LibVLC.
  /// On later versions, that is optional and unnecessary.
  OSVERSIONINFO osvi{sizeof(OSVERSIONINFO)};
  GetVersionEx(&osvi);
  if (6 > osvi.dwMajorVersion ||
      6 == osvi.dwMajorVersion && 0 == osvi.dwMinorVersion) {
    SetErrorMode(SEM_FAILCRITICALERRORS);
  }
  /* Load the VLC engine */
  inst = libvlc_new(0, NULL);

  /* Create a new item */
  m = libvlc_media_new_location(inst, url);
  /// Test code, set proxy here ;Should be removed
  // 	libvlc_media_add_option(m, ":http-proxy=http://192.168.40.1:9999");
  // m = libvlc_media_new_path (inst, "/path/to/test.mov");

  /* Create a media player playing environement */
  mp = libvlc_media_player_new_from_media(m);

  /* No need to keep the media now */
  libvlc_media_release(m);

#if 0
	/* This is a non working code that show how to hooks into a window,
	* if we have a window around */
	libvlc_media_player_set_xwindow(mp, xid);
	/* or on windows */
	libvlc_media_player_set_hwnd(mp, hwnd);
	/* or on mac os */
	libvlc_media_player_set_nsobject(mp, view);
#endif

  /* play the media_player */
  libvlc_media_player_play(mp);

  Sleep(15 * 1000); /* Let it play a bit */

  /* Stop playing */
  libvlc_media_player_stop(mp);

  /* Free the media_player */
  libvlc_media_player_release(mp);

  libvlc_release(inst);

  return 0;
}

int main(void) {
  vlc_url_test("http://www.w3school.com.cn/i/movie.mp4");
  return 0;
}

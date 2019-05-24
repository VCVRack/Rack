//
//    file: test_zenity.c
//  author: bsp
// license: CC0 (public domain)
// created: 24May2019
// changed:
//

#include <stdio.h>
#include <stdlib.h>

#include "osdialog.h"

int main(int argc, char**argv) {
   int r = 0;
   char *name = NULL;

   if(1 == argc)
   {
      // Test message dialogs (ok)
      r = osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, "This is an info message.");
      printf("info dialog returned r=%d\n", r);

      r = osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, "This is a warning message.");
      printf("warning dialog returned r=%d\n", r);

      r = osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "This is an error message.");
      printf("error dialog returned r=%d\n", r);

      // Test message dialogs (ok / cancel / yes / no)
      r = osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "This is an info message.");
      printf("info dialog returned r=%d\n", r);

      r = osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "This is a warning message.");
      printf("warning dialog returned r=%d\n", r);

      r = osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "This is an error message.");
      printf("error dialog returned r=%d\n", r);
   }

   // Test file dialogs
   if(argc <= 2)
   {
      osdialog_filter_patterns patternsMP3 = {
         "mp3",
         NULL
      };
      osdialog_filter_patterns patternsFLAC = {
         "flac",
         &patternsMP3
      };
      osdialog_filter_patterns patternsWAV = {
         "wav",
         &patternsFLAC
      };
      osdialog_filters filterAudio = {
         "Audio Files",
         &patternsWAV,
         NULL
      };

      osdialog_filter_patterns patternsAVI = {
         "avi",
         NULL
      };
      osdialog_filter_patterns patternsMP4 = {
         "mp4",
         &patternsAVI
      };
      osdialog_filter_patterns patternsMKV = {
         "mkv",
         &patternsMP4
      };
      osdialog_filters filterVideo = {
         "Video Files",
         &patternsMKV,
         &filterAudio
      };

      // File selection (open)
      {
         name = osdialog_file(OSDIALOG_OPEN, "/tmp/"/*path*/, "myfile", &filterVideo);
         if(NULL != name)
         {
            printf("selected file (open) is \"%s\"\n", name);
            free(name);
         }
         else
         {
            printf("file OSDIALOG_OPEN was canceled.\n");
         }
      }

      // Directory selection
      {
         name = osdialog_file(OSDIALOG_OPEN_DIR, "/tmp/"/*path*/, "myfile", NULL/*filters*/);
         if(NULL != name)
         {
            printf("selected directory is \"%s\"\n", name);
            free(name);
         }
         else
         {
            printf("file OSDIALOG_OPEN_DIR was canceled.\n");
         }
      }

      // File selection (save)
      {
         name = osdialog_file(OSDIALOG_SAVE, "/tmp/"/*path*/, "myfile", &filterVideo);
         if(NULL != name)
         {
            printf("selected file (save) is \"%s\"\n", name);
            free(name);
         }
         else
         {
            printf("file OSDIALOG_SAVE was canceled.\n");
         }
      }

   }

   // Test color dialog
   if(argc <= 3)
   {
      osdialog_color color = { 80/*r*/, 120/*g*/, 160/*b*/, 200/*a*/ };
      if(osdialog_color_picker(&color, 0/*opacity*/))
      {
         printf("selected color rgba=(%d, %d, %d, %d) (#%02x%02x%02x%02x)\n",
                color.r, color.g, color.b, color.a,
                color.a, color.r, color.g, color.b
                );
      }
      else
      {
         printf("color picker dialog was canceled.\n");
      }
   }

   return r;
}

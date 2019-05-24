//
//    file: osdialog_zenity.cpp
//  author: bsp
// license: CC0 (public domain)
// created: 24May2019
// changed:
//

// Enable / disable debug messages
#define Dprintf if(1);else printf

#include <string>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "osdialog.h"

#define ZENITY_CMD "/usr/bin/zenity"
#define DLG_TITLE  "Rack"

#define MAX_BUF_SIZE  (65536)
#ifndef NULL
#define NULL ((void*)0)
#endif // NULL


/* @function psystem,String cmd,char access,String buf:int
Invoke system command

@arg cmd Command line
@arg buf Input resp. output buffer
@return Number of bytes read resp. written
*/
static int loc_psystem(const char *_cmd, std::string &_buf) { 
   int r = 0;

   Dprintf("osdialog_zenity: loc_psystem(cmd=\'%s\')\n", _cmd);

   char *buf = new char[MAX_BUF_SIZE];
   _buf.clear();
   if(NULL != buf)
   {
      FILE *f; 
#ifdef _MSC_VER
      f=::_popen(_cmd, "r"); 
#else 
      f=::popen(_cmd, "r"); 
#endif 
      int fe = ferror(f); 
      if(NULL != f) 
      { 
         r = (int)::fread(buf, 1, MAX_BUF_SIZE-1, f); 
         fe = ferror(f); 
         if( ((unsigned int)r+1) < (MAX_BUF_SIZE-1) )
         {
            buf[r] = 0;
            _buf = buf;
         }
#ifdef _MSC_VER
         ::_pclose(f); 
#else 
         ::pclose(f); 
#endif 
      } 
      if( (0 != fe) || (NULL == f) ) 
      { 
         printf("[---] osdialog_zenity: psystem(%s, \'r\', ..) failed with error code %i.\n", 
                _cmd,
                fe 
                ); 
      } 

      delete [] buf;
   } // if buf

   return r; 
} 

static int loc_system(const char *_cmd) { 
   Dprintf("osdialog_zenity: loc_system(cmd=\'%s\')\n", _cmd);
   int r = ::system(_cmd); 
   return r;
} 


extern "C" {
int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
   int r = 0;

   std::string cmd = ZENITY_CMD;

   if(OSDIALOG_OK == buttons)
   {
      switch(level)
      {
         default:
         case OSDIALOG_INFO:
            cmd.append(" --info");
            break;

         case OSDIALOG_WARNING:
            cmd.append(" --warning");
            break;

         case OSDIALOG_ERROR:
            cmd.append(" --error");
            break;
      }
   }
   else
   {
      cmd.append(" --question");

      switch(level)
      {
         default:
         case OSDIALOG_INFO:
            cmd.append(" --icon-name \"dialog-information\"");
            break;

         case OSDIALOG_WARNING:
            cmd.append(" --icon-name \"dialog-warning\"");
            break;

         case OSDIALOG_ERROR:
            cmd.append(" --icon-name \"dialog-error\"");
            break;
      }
   }

   cmd.append(" --text=\"");
   cmd.append(message);
   cmd.append("\"");

   cmd.append(" --title \"" DLG_TITLE "\"");

   r = loc_system(cmd.c_str());
   r = (256 == r) ? 0 : 1;

   return r;
}

char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
   char *r = NULL;

   std::string cmd = ZENITY_CMD;

   cmd.append(" --file-selection");

   switch(action)
   {
      default:
      case OSDIALOG_OPEN:
         break;

      case OSDIALOG_OPEN_DIR:
         cmd.append("  --directory");
         break;

      case OSDIALOG_SAVE:
         cmd.append("  --save --confirm-overwrite");
         break;
   }

   osdialog_filters *cf = filters;
   while(NULL != cf)
   {
      cmd.append(" --file-filter \"");
      cmd.append(cf->name);
      cmd.append(" (");
      osdialog_filter_patterns *cp = cf->patterns;
      while(NULL != cp)
      {
         if(cp != cf->patterns)
            cmd.append(", ");
         cmd.append(cp->pattern);
         cp = cp->next;
      }
      cmd.append(") | ");
      
      cp = cf->patterns;
      while(NULL != cp)
      {
         cmd.append("*.");
         cmd.append(cp->pattern);
         cp = cp->next;
         if(NULL != cp)
            cmd.append(" ");
      }

      cmd.append("\"");
      cf = cf->next;
   }

   cmd.append(" --filename \"");
   if(NULL != path)
   {
      cmd.append(path);
      cmd.append("/");
   }
   if(NULL != filename)
   {
      cmd.append(filename);
   }
   cmd.append("\"");

   cmd.append(" --title \"" DLG_TITLE "\"");

   std::string buf;
   if(loc_psystem(cmd.c_str(), buf) > 1)
   {
      Dprintf("osdialog_zenity: file selection returned buf=\'%s\'\n", buf.c_str());
      size_t bufSize = buf.size() - 1;

      r = (char*)malloc((bufSize + 1) * sizeof(char));
      if(NULL != r)
      {
         memcpy((void*)r, (void*)buf.c_str(), bufSize);
         r[bufSize] = 0;
      }
   }

   return r;
}

int osdialog_color_picker(osdialog_color *color, int opacity) {
   int r = 0;

   (void)opacity;  // 1=enable opacity slider

   std::string cmd = ZENITY_CMD;

   cmd.append(" --color-selection");

   if(NULL != color)
   {
      cmd.append(" --color \"rgba(");
      char buf[10];
      sprintf(buf, "%d,%d,%d,%f", color->r, color->g, color->b, color->a / 255.0f);
      cmd.append(buf);
      cmd.append(")\"");
   }

   cmd.append(" --title \"" DLG_TITLE "\"");
#if 0
   cmd.append(" --show-palette");
#endif

   std::string buf;
   if(loc_psystem(cmd.c_str(), buf) > 1)
   {
      Dprintf("osdialog_zenity: color selection returned buf=\'%s\'\n", buf.c_str());
      {
         unsigned int r = 0u, g = 0u, b = 0u;
         float a = 1.0f;
         if(NULL != strstr(buf.c_str(), "rgba("))
         {
            // (note) not supported by Zenity 3.22.0 (latest version as of 24May2019)
            sscanf(buf.c_str(), "rgba(%u,%u,%u,%f)", &r, &g, &b, &a);
            color->r = (unsigned char)(r);
            color->g = (unsigned char)(g);
            color->b = (unsigned char)(b);
            color->a = (unsigned char)(a * 255u);
         }
         else
         {
            sscanf(buf.c_str(), "rgb(%u,%u,%u)", &r, &g, &b);
            color->r = (unsigned char)(r);
            color->g = (unsigned char)(g);
            color->b = (unsigned char)(b);
         }
      }
      r = 1;
   }

   return r;
}
} // extern "C"


#include <stdio.h>
#include "Template.hpp"
// extern "C" extern int glfwInit(void);

RACK_PLUGIN_MODEL_DECLARE(Template_shared, MyModule);

RACK_PLUGIN_INIT(Template_shared) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://<your_website>");
   RACK_PLUGIN_INIT_MANUAL("https://<link_to_the_manual>");

   // Add all Models defined throughout the plugin
   RACK_PLUGIN_MODEL_ADD(Template_shared, MyModule);

   // Any other plugin initialization may go here.
   // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
   printf("hello from Template_shared plugin!\n");

   // glfwInit();

}

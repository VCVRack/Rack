#include "Southpole.hpp"

RACK_PLUGIN_MODEL_DECLARE(Southpole, Abr);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Annuli);   
RACK_PLUGIN_MODEL_DECLARE(Southpole, Aux);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Balaclava);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Bandana);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank1HP); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank2HP); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank4HP); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank8HP); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank16HP);   
RACK_PLUGIN_MODEL_DECLARE(Southpole, Blank42HP);   
RACK_PLUGIN_MODEL_DECLARE(Southpole, But);
RACK_PLUGIN_MODEL_DECLARE(Southpole, CornrowsX);
RACK_PLUGIN_MODEL_DECLARE(Southpole, DeuxEtageres); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Etagere);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Falls);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Ftagn);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Fuse);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Gnome);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Piste);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Pulse);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Rakes);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Riemann); 
RACK_PLUGIN_MODEL_DECLARE(Southpole, Smoke);  
RACK_PLUGIN_MODEL_DECLARE(Southpole, Snake);  
RACK_PLUGIN_MODEL_DECLARE(Southpole, Sns);
RACK_PLUGIN_MODEL_DECLARE(Southpole, Splash);    
RACK_PLUGIN_MODEL_DECLARE(Southpole, Sssh);   
RACK_PLUGIN_MODEL_DECLARE(Southpole, Wriggle);

RACK_PLUGIN_INIT(Southpole) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/gbrandt1/southpole-vcvrack");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/gbrandt1/southpole-vcvrack/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(Southpole, Abr);
   RACK_PLUGIN_MODEL_ADD(Southpole, Annuli);   
   RACK_PLUGIN_MODEL_ADD(Southpole, Aux);
   RACK_PLUGIN_MODEL_ADD(Southpole, Balaclava);
   RACK_PLUGIN_MODEL_ADD(Southpole, Bandana);
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank1HP); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank2HP); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank4HP); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank8HP); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank16HP);   
   RACK_PLUGIN_MODEL_ADD(Southpole, Blank42HP);   
   RACK_PLUGIN_MODEL_ADD(Southpole, But);
   RACK_PLUGIN_MODEL_ADD(Southpole, CornrowsX);
   RACK_PLUGIN_MODEL_ADD(Southpole, DeuxEtageres); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Etagere);
   RACK_PLUGIN_MODEL_ADD(Southpole, Falls);
   RACK_PLUGIN_MODEL_ADD(Southpole, Ftagn);
   RACK_PLUGIN_MODEL_ADD(Southpole, Fuse);
   RACK_PLUGIN_MODEL_ADD(Southpole, Gnome);
   RACK_PLUGIN_MODEL_ADD(Southpole, Piste);
   RACK_PLUGIN_MODEL_ADD(Southpole, Pulse);
   RACK_PLUGIN_MODEL_ADD(Southpole, Rakes);
   RACK_PLUGIN_MODEL_ADD(Southpole, Riemann); 
   RACK_PLUGIN_MODEL_ADD(Southpole, Smoke);  
   RACK_PLUGIN_MODEL_ADD(Southpole, Snake);  
   RACK_PLUGIN_MODEL_ADD(Southpole, Sns);
   RACK_PLUGIN_MODEL_ADD(Southpole, Splash);    
   RACK_PLUGIN_MODEL_ADD(Southpole, Sssh);   
   RACK_PLUGIN_MODEL_ADD(Southpole, Wriggle);
}

#include "mtsch.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    // The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
    // It must only contain letters, numbers, and characters "-" and "_". No spaces.
    // To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
    // The ZIP package must only contain one folder, with the name equal to the plugin's slug.
    p->slug = "mtsch-plugins";
#ifdef VERSION
    p->version = TOSTRING(VERSION);
#endif
    p->website = "https://github.com/mtsch/mtsch-vcvrack-plugins";
    p->manual = "https://github.com/mtsch/mtsch-vcvrack-plugins";

    // For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
    p->addModel(modelSum);
    p->addModel(modelRationals);
    p->addModel(modelTriggerPanic);
    //p->addModel(createModel<TriggerPanicWidget>("mtsch", "TriggerPanic", "Trigger Panic!", DELAY_TAG));

    // Any other plugin initialization may go here.
    // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

#!/usr/bin/env python3

import sys
import os
import re
import json
import xml.etree.ElementTree


if sys.version_info < (3, 6):
	print("Python 3.6 or higher required")
	exit(1)


class UserException(Exception):
    pass


def input_default(prompt, default=""):
	str = input(f"{prompt} [{default}]: ")
	if str == "":
		return default
	return str


def is_valid_slug(slug):
	return re.match(r'^[a-zA-Z0-9_\-]+$', slug) != None


def slug_to_identifier(slug):
	if len(slug) == 0 or slug[0].isdigit():
		slug = "_" + slug
	slug = slug[0].upper() + slug[1:]
	slug = slug.replace('-', '_')
	return slug


def usage(script):
	text = f"""Usage: {script} <command> ...
Run commands without arguments for command help.

Commands:
  createplugin <slug>
  createmodule <module slug>
  createmanifest
"""
	print(text)


def usage_create_plugin(script):
	text = f"""Usage: {script} createplugin <slug>

A directory <slug> will be created in the current working directory and seeded with initial files.
"""
	print(text)


def create_plugin(slug):
	# Check slug
	if not is_valid_slug(slug):
		raise UserException("Slug must only contain ASCII letters, numbers, '-', and '_'.")

	# Check if plugin directory exists
	plugin_dir = os.path.join(slug, '')
	if os.path.exists(plugin_dir):
		raise UserException(f"Directory {plugin_dir} already exists")

	# Query manifest information
	manifest = {}
	manifest['slug'] = slug
	manifest['name'] = input_default("Plugin name", slug)
	manifest['version'] = input_default("Version", "1.0.0")
	manifest['license'] = input_default("License (if open-source, use license identifier from https://spdx.org/licenses/)", "proprietary")
	manifest['author'] = input_default("Author")
	manifest['authorEmail'] = input_default("Author email (optional)")
	manifest['authorUrl'] = input_default("Author website URL (optional)")
	manifest['pluginUrl'] = input_default("Plugin website URL (optional)")
	manifest['manualUrl'] = input_default("Manual website URL (optional)")
	manifest['sourceUrl'] = input_default("Source code URL (optional)")
	manifest['donateUrl'] = input_default("Donate URL (optional)")
	manifest['modules'] = []

	# Create plugin directory
	os.mkdir(plugin_dir)

	# Dump JSON
	manifest_filename = os.path.join(plugin_dir, 'plugin.json')
	with open(manifest_filename, "w") as f:
		json.dump(manifest, f, indent="\t")
	print(f"Manifest created at {manifest_filename}")

	# Create subdirectories
	os.mkdir(os.path.join(plugin_dir, "src"))
	os.mkdir(os.path.join(plugin_dir, "res"))

	# Create Makefile
	makefile = """# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
"""
	with open(os.path.join(plugin_dir, "Makefile"), "w") as f:
		f.write(makefile)

	# Create plugin.hpp
	plugin_hpp = """#include "rack.hpp"


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
"""
	with open(os.path.join(plugin_dir, "src/plugin.hpp"), "w") as f:
		f.write(plugin_hpp)

	# Create plugin.cpp
	plugin_cpp = """#include "plugin.hpp"


Plugin *pluginInstance;


void init(Plugin *p) {
	pluginInstance = p;

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
"""
	with open(os.path.join(plugin_dir, "src/plugin.cpp"), "w") as f:
		f.write(plugin_cpp)

	git_ignore = """/build
/dist
/plugin.so
/plugin.dylib
/plugin.dll
.DS_Store
"""
	with open(os.path.join(plugin_dir, ".gitignore"), "w") as f:
		f.write(git_ignore)

	print(f"Created template plugin in {plugin_dir}")
	os.system(f"cd {plugin_dir} && git init")
	print(f"You may use `make`, `make clean`, `make dist`, `make install`, etc in the {plugin_dir} directory.")


def usage_create_module(script):
	text = f"""Usage: {script} createmodule <module slug>

Must be called in a plugin directory.
A panel file must exist in res/<module slug>.svg.
A source file will be created at src/<module slug>.cpp.

Instructions for creating a panel:
- Only Inkscape is supported by this script and Rack's SVG renderer.
- Create a document with units in "mm", height of 128.5 mm, and width of a multiple of 5.08 mm (1 HP in Eurorack).
- Design the panel.
- Create a layer named "widgets".
- For each component, create a shape on the widgets layer.
	- Use a circle to place a component by its center.
		The size of the circle does not matter, only the center point.
		A `create*Centered()` call is generated in C++.
	- Use a rectangle to to place a component by its top-left point.
		This should only be used when the shape's size is equal to the component's size in C++.
		A `create*()` call is generated in C++.
- Set the color of each shape depending on the component's type.
	- Param: red #ff0000
	- Input: green #00ff00
	- Output: blue #0000ff
	- Light: magenta #ff00ff
	- Custom widgets: yellow #ffff00
- Hide the widgets layer and save to res/<module slug>.svg.
"""
	print(text)


def create_module(slug):
	# Check slug
	if not is_valid_slug(slug):
		raise UserException("Slug must only contain ASCII letters, numbers, '-', and '_'.")

	# Read manifest
	manifest_filename = 'plugin.json'
	manifest = None
	with open(manifest_filename, "r") as f:
		manifest = json.load(f)

	# Check if module manifest exists
	module_manifests = filter(lambda m: m['slug'] == slug, manifest['modules'])
	if module_manifests:
		# Add module to manifest
		module_manifest = {}
		module_manifest['slug'] = slug
		module_manifest['name'] = input_default("Module name", slug)
		module_manifest['description'] = input_default("One-line description (optional)")
		tags = input_default("Tags (comma-separated, see https://github.com/VCVRack/Rack/blob/v1/src/plugin.cpp#L543 for list)")
		tags = tags.split(",")
		tags = [tag.strip() for tag in tags]
		if len(tags) == 1 and tags[0] == "":
			tags = []
		module_manifest['tags'] = tags

		manifest['modules'].append(module_manifest)

		# Write manifest
		with open(manifest_filename, "w") as f:
			json.dump(manifest, f, indent="\t")

		print(f"Added {slug} to plugin.json")

	# Check filenames
	panel_filename = f"res/{slug}.svg"
	source_filename = f"src/{slug}.cpp"

	if os.path.exists(source_filename):
		if input_default(f"{source_filename} already exists. Overwrite?", "n").lower() != "y":
			return

	# Read SVG XML
	tree = None
	try:
		tree = xml.etree.ElementTree.parse(panel_filename)
	except FileNotFoundError:
		raise UserException(f"Panel not found at {panel_filename}")

	components = panel_to_components(tree)
	print(f"Components extracted from {panel_filename}")

	# Write source
	source = components_to_source(components, slug)

	with open(source_filename, "w") as f:
		f.write(source)
	print(f"Source file generated at {source_filename}")


def panel_to_components(tree):
	ns = {
		"svg": "http://www.w3.org/2000/svg",
		"inkscape": "http://www.inkscape.org/namespaces/inkscape",
	}

	# Get widgets layer
	root = tree.getroot()
	groups = root.findall(".//svg:g[@inkscape:label='widgets']", ns)
	if len(groups) < 1:
		raise UserException("Could not find \"widgets\" layer on panel")

	# Get circles and rects
	widgets_group = groups[0]
	circles = widgets_group.findall(".//svg:circle", ns)
	rects = widgets_group.findall(".//svg:rect", ns)

	components = {}
	components['params'] = []
	components['inputs'] = []
	components['outputs'] = []
	components['lights'] = []
	components['widgets'] = []

	for el in circles + rects:
		c = {}
		# Get name
		name = el.get('inkscape:label')
		if name is None:
			name = el.get('id')
		name = slug_to_identifier(name).upper()
		c['name'] = name

		# Get color
		style = el.get('style')
		color_match = re.search(r'fill:\S*#(.{6});', style)
		color = color_match.group(1)
		c['color'] = color

		# Get position
		if el.tag == "{http://www.w3.org/2000/svg}rect":
			x = float(el.get('x'))
			y = float(el.get('y'))
			width = float(el.get('width'))
			height = float(el.get('height'))
			c['x'] = round(x, 3)
			c['y'] = round(y, 3)
			c['width'] = round(width, 3)
			c['height'] = round(height, 3)
		elif el.tag == "{http://www.w3.org/2000/svg}circle":
			cx = float(el.get('cx'))
			cy = float(el.get('cy'))
			c['cx'] = round(cx, 3)
			c['cy'] = round(cy, 3)

		if color == 'ff0000':
			components['params'].append(c)
		if color == '00ff00':
			components['inputs'].append(c)
		if color == '0000ff':
			components['outputs'].append(c)
		if color == 'ff00ff':
			components['lights'].append(c)
		if color == 'ffff00':
			components['widgets'].append(c)

	return components


def components_to_source(components, slug):
	identifier = slug_to_identifier(slug)
	source = ""

	source += f"""#include "plugin.hpp"


struct {identifier} : Module {{"""

	# Params
	source += """
	enum ParamIds {"""
	for c in components['params']:
		source += f"""
		{c['name']}_PARAM,"""
	source += """
		NUM_PARAMS
	};"""

	# Inputs
	source += """
	enum InputIds {"""
	for c in components['inputs']:
		source += f"""
		{c['name']}_INPUT,"""
	source += """
		NUM_INPUTS
	};"""

	# Outputs
	source += """
	enum OutputIds {"""
	for c in components['outputs']:
		source += f"""
		{c['name']}_OUTPUT,"""
	source += """
		NUM_OUTPUTS
	};"""

	# Lights
	source += """
	enum LightIds {"""
	for c in components['lights']:
		source += f"""
		{c['name']}_LIGHT,"""
	source += """
		NUM_LIGHTS
	};"""


	source += f"""

	{identifier}() {{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);"""

	for c in components['params']:
		source += f"""
		params[{c['name']}_PARAM].config(0.f, 1.f, 0.f, "");"""

	source += """
	}

	void process(const ProcessArgs &args) override {
	}
};"""

	source += f"""

struct {identifier}Widget : ModuleWidget {{
	{identifier}Widget({identifier} *module) {{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/{slug}.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));"""


	# Params
	if len(components['params']) > 0:
		source += "\n"
	for c in components['params']:
		if 'cx' in c:
			source += f"""
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_PARAM));"""
		else:
			source += f"""
		addParam(createParam<RoundBlackKnob>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_PARAM));"""

	# Inputs
	if len(components['inputs']) > 0:
		source += "\n"
	for c in components['inputs']:
		if 'cx' in c:
			source += f"""
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_INPUT));"""
		else:
			source += f"""
		addInput(createInput<PJ301MPort>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_INPUT));"""

	# Outputs
	if len(components['outputs']) > 0:
		source += "\n"
	for c in components['outputs']:
		if 'cx' in c:
			source += f"""
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_OUTPUT));"""
		else:
			source += f"""
		addOutput(createOutput<PJ301MPort>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_OUTPUT));"""

	# Lights
	if len(components['lights']) > 0:
		source += "\n"
	for c in components['lights']:
		if 'cx' in c:
			source += f"""
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_LIGHT));"""
		else:
			source += f"""
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_LIGHT));"""

	# Widgets
	if len(components['widgets']) > 0:
		source += "\n"
	for c in components['widgets']:
		if 'cx' in c:
			source += f"""
		addChild(createWidgetCentered<Widget>(mm2px(Vec({c['cx']}, {c['cy']}))));"""
		else:
			source += f"""
		// mm2px(Vec({c['width']}, {c['height']}))
		addChild(createWidget<Widget>(mm2px(Vec({c['x']}, {c['y']}))));"""

	source += f"""
	}}
}};


Model *model{identifier} = createModel<{identifier}, {identifier}Widget>("{slug}");"""

	return source


def parse_args(args):
	if len(args) >= 2:
		if args[1] == 'createplugin':
			if len(args) >= 3:
				create_plugin(args[2])
				return
			usage_create_plugin(args[0])
			return
		if args[1] == 'createmodule':
			if len(args) >= 3:
				create_module(args[2])
				return
			usage_create_module(args[0])
			return
	usage(args[0])


if __name__ == "__main__":
	try:
		parse_args(sys.argv)
	except KeyboardInterrupt:
		pass
	except UserException as e:
		print(e)
		sys.exit(1)

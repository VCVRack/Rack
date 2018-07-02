import sys
import json
import os

for filename in sys.argv[1:]:
	slug = os.path.splitext(os.path.basename(filename))[0]
	with open(filename, "r") as f:
		manifest = json.load(f)

	if os.path.exists("repos/" + slug):
		manifest['status'] = "available"
	else:
		next

	with open(filename, "w") as f:
		json.dump(manifest, f, indent="  ")

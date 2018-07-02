import sys
import json

for filename in sys.argv[1:]:
	with open(filename, "r") as f:
		manifest1 = json.load(f)
	manifest2 = {}
	if 'name' in manifest1:
		manifest2['name'] = manifest1['name']
	if 'author' in manifest1:
		manifest2['author'] = manifest1['author']
	if 'license' in manifest1:
		manifest2['license'] = manifest1['license']
	if 'homepage' in manifest1:
		manifest2['websiteUrl'] = manifest1['homepage']
	if 'manual' in manifest1:
		manifest2['manualUrl'] = manifest1['manual']
	if 'source' in manifest1:
		manifest2['sourceUrl'] = manifest1['source']
	if 'donation' in manifest1:
		manifest2['donateUrl'] = manifest1['donation']
	if 'version' in manifest1:
		manifest2['latestVersion'] = manifest1['version']
	if 'productId' in manifest1:
		manifest2['productId'] = manifest1['productId']
	with open(filename, "w") as f:
		json.dump(manifest2, f, indent="  ")

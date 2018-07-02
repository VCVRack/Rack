import glob
import json
import time
import os
import sys


RACK_SDK = os.path.abspath("Rack-SDK")


def system(cmd):
	if os.system(cmd):
		raise Exception(f"Failed command: {cmd}")


def build_mac(repo):
	env = f'CC=x86_64-apple-darwin15-clang CXX=x86_64-apple-darwin15-clang++-libc++ STRIP=x86_64-apple-darwin15-strip RACK_DIR={RACK_SDK}'
	make = f'{env} make -j$(nproc) -C {repo}'
	system(f'{make} clean')
	system(f'{make} dist')


def build_win(repo):
	env = f'CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ STRIP=x86_64-w64-mingw32-strip RACK_DIR={RACK_SDK}'
	make = f'{env} make -j$(nproc) -C {repo}'
	system(f'{make} clean')
	system(f'{make} dist')


def build_lin(repo):
	env = f'-e RACK_DIR=/Rack-SDK'
	make = f'make -j$(nproc)'
	repo_abs = os.path.abspath(repo)
	system(f'docker run --rm -v {RACK_SDK}:/Rack-SDK -v {repo_abs}:/workdir -w /workdir -u vortico {env} a0b9c87ec456 {make} clean')
	system(f'docker run --rm -v {RACK_SDK}:/Rack-SDK -v {repo_abs}:/workdir -w /workdir -u vortico {env} a0b9c87ec456 {make} dist')


def move_package(repo, slug):
	system('mkdir -p downloads')
	system(f'mv {repo}/dist/{slug}-*.zip downloads/')


def delete_package(slug):
	system(f'rm -f downloads/{slug}-*.zip')



repos = sys.argv[1:]
force_update = True
if not repos:
	force_update = False
	repos = glob.glob("repos/*")

built_repos = []

for repo in repos:
	slug = os.path.basename(repo)
	manifest_filename = "manifests/" + slug + ".json"
	with open(manifest_filename, "r") as f:
		manifest = json.load(f)

	# We need a repoVersion to build
	if 'repoVersion' not in manifest:
		continue
	# Skip if update is not needed
	if not force_update and 'repoVersion' in manifest and 'latestVersion' in manifest and manifest['latestVersion'] == manifest['repoVersion']:
		continue

	# Build repo
	try:
		build_lin(repo)
		move_package(repo, slug)
		build_win(repo)
		move_package(repo, slug)
		build_mac(repo)
		move_package(repo, slug)
	except Exception as e:
		print(e)
		input("Enter to proceed")
		delete_package(slug)
		continue

	built_repos.append(slug)

	# Update build information
	manifest['latestVersion'] = manifest['repoVersion']
	manifest['buildTimestamp'] = round(time.time())
	manifest['status'] = "available"

	with open(manifest_filename, "w") as f:
		json.dump(manifest, f, indent="  ")


if built_repos:
	print()
	print("Built " + ", ".join(built_repos))
else:
	print("Nothing to build")

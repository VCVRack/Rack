# check for the community repo locally
# RUN IN plugins directory inside Rack.
# USAGE grabPlugins.sh <target>
# Where <target> is one of win, lin, or mac
if [ $# -gt 0 ]
then
    platform="${1}"
else
    platform=mac
fi

if [ -d "community" ]; then
    pushd community
    # discard any changes
    git reset HEAD --hard

    # update the community repo if it exists
    git pull
    popd
else
    # community repo does not exist so pull it down
    git clone https://github.com/VCVRack/community
fi

for gitPlugin in $(cat community/plugins/*.json | grep 'mac.zip' | awk -F'"' '{ print $4}')
do
    zipname="$(echo $gitPlugin | sed -e 's@.*/@@' | sed -e 's/\?raw=true//')"
    curl -L "${gitPlugin}" -o "${zipname}"
    if [[ -f ${zipname} ]]
    then
	unzip  -o "${zipname}"
	rm "${zipname}"
    fi
done
		 
if [ "${platform}" != "mac" ]
then
    rm -fr __MACOSX
fi

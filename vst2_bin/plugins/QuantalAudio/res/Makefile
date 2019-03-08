
INKSCAPE=inkscape
SVGO=svgo
SVGS=MasterMixer.svg BufferedMult.svg UnityMix.svg DaisyChannel.svg DaisyMaster.svg Horsehair.svg

all: $(SVGS)

MasterMixer.svg: src/MasterMixer.src.svg
	$(INKSCAPE) src/MasterMixer.src.svg --export-plain-svg=MasterMixer-textpaths.svg --export-text-to-path
	$(SVGO) -i MasterMixer-textpaths.svg -o MasterMixer.svg
	rm MasterMixer-textpaths.svg

BufferedMult.svg: src/BufferedMult.src.svg
	$(INKSCAPE) src/BufferedMult.src.svg --export-plain-svg=BufferedMult-textpaths.svg --export-text-to-path
	$(SVGO) -i BufferedMult-textpaths.svg -o BufferedMult.svg
	rm BufferedMult-textpaths.svg

UnityMix.svg: src/UnityMix.src.svg
	$(INKSCAPE) src/UnityMix.src.svg --export-plain-svg=UnityMix-textpaths.svg --export-text-to-path
	$(SVGO) -i UnityMix-textpaths.svg -o UnityMix.svg
	rm UnityMix-textpaths.svg

DaisyChannel.svg: src/DaisyChannel.src.svg
	$(INKSCAPE) src/DaisyChannel.src.svg --export-plain-svg=DaisyChannel-textpaths.svg --export-text-to-path
	$(SVGO) -i DaisyChannel-textpaths.svg -o DaisyChannel.svg
	rm DaisyChannel-textpaths.svg

DaisyMaster.svg: src/DaisyMaster.src.svg
	$(INKSCAPE) src/DaisyMaster.src.svg --export-plain-svg=DaisyMaster-textpaths.svg --export-text-to-path
	$(SVGO) -i DaisyMaster-textpaths.svg -o DaisyMaster.svg
	rm DaisyMaster-textpaths.svg

Horsehair.svg: src/Horsehair.src.svg
	$(INKSCAPE) src/Horsehair.src.svg --export-plain-svg=Horsehair-textpaths.svg --export-text-to-path
	$(SVGO) -i Horsehair-textpaths.svg -o Horsehair.svg
	rm Horsehair-textpaths.svg

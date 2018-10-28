# Functional VCO-1 <a name="fun"></a>

![Functional image](../docs/functional.png)

Functional VCO-1 works just like its namesake. The control layout is familiar, the sound is the same, but it uses about 1/4 as much CPU as the original.

We believe VCV's Fundamental VCO is an unsung hero. It's one of the few VCOs that never has audible aliasing artifacts. You can sync it, and modulate all its inputs, but the sound never falls apart.

We "forked" the code to Fundamental VCO-1 and modified it a little bit to make it much more CPU efficient. Now you may use a lot more of them without pops, clicks, and dropouts.

If you would like the details of how we did this optimization, you can [find them here](../docs/vco-optimization.md).

We have an informational article that talks more about aliasing. It shows you how to compare different modules using a spectrum analyzer. [Aliasing Story](./aliasing.md).

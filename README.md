# ColorTransfert

C++ implementation of "Color Transfer between Images" by Reinhard et al.

The code uses GLM to do basic linear algebra, and stb_image/stb_image_write to open and create images.

Executable added, so should be easy to use.
To build the code, with VS, it should be trivial, without it, even more.

To test it, run in terminal: 

```bash
ColorTransform.exe sourceImage targetImage outputImage.PNG
```

Note the source and target image can be in any reasonable format, but the generated image will be a png.

Example:

![Source image](ColorTransfert/source.jpg) 
![Target image](ColorTransfert/target.jpg) 
![Output image](ColorTransfert/output.png)

for %%f in (*.png) DO ( convert "%%f" -strip -resize 128x128! "%%f" )

# Panoptic-Generator
Deep Learning Panoptic Sample Generator


This module converts GIS data into panoptic segmentation tiles.

The software inputs consider:
- original image: original satellite image in ENVI or TIF format. If the problem is dealing with multispectral or multitemporal images (many channels), ENVI data is preferable.
- sequential image: image file in which all "thing" categories present a unique value. The best way to gather this data is to put all "stuff" categories first, e.g., if there are three "stuff" categories, use labels 1, 2, and 3. For all other categories (i.e., the "thing" categories), apply a sequential generator value for each polygon.
- Semantic image: image file in which all categories present a unique label (traditional ground truth for semantic segmentation).
- Point shapefiles: using the ArcMap software, the user may choose the locations in which the software will crop the image tiles. The point shapefile will be the centroid of the square tile. A tip in this process is to use the GraphicBuffer tool on all chosen point shapefiles to see the final tile.

The output file is a folder containing:
- original image tiles
- panoptic image tiles
- semantic image tiles
- a folder with JSON annotations in the COCO format

Besides, suppose the user selects point shapefiles for training, validation, and testing. In that case, there will be ten folders (3 for original image tiles, 3 for panoptic image tiles, 3 for semantic image tiles, and one for all JSON annotations). The user must choose at least one Point Shapefile for the training set.

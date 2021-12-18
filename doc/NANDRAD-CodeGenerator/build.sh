#!/bin/bash

ADOC=NANDRAD-CodeGenerator

echo '*** Generating html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&
asciidoctor -a lang=en -r asciidoctor-mathematical $ADOC.adoc &&

echo '*** Generating pdf ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=en  -a pdf-theme=../adoc_utils/pdf-theme.yml -r asciidoctor-mathematical  -r ../adoc_utils/rouge_theme.rb -a pdf-fontsdir="../adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
python ../adoc_utils/scripts/adoc-image-prep.py html . &&

echo '*** Copying files to ../../docs directory' &&

if [ ! -d "../../docs/$ADOC" ]; then
	mkdir ../../docs/$ADOC
fi &&
mv $ADOC.html ../../docs/$ADOC/index.html 
#mv $ADOC.pdf ../../docs &&

#imgFiles=(./images/*.png) 
#if [ ${#imgFiles[@]} -gt 0 ]; then
#	echo 'Copying '${#imgFiles[@]}' images to ../../docs/'$ADOC'/images' &&
#	cp -r ./images/*.png ../../docs/$ADOC/images
#fi


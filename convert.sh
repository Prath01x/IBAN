# A utility to help you test your own images
# This script converts images to ppm (pnm format 3, ascii, 8bit color depth).

# How to do it manually:
# * GIMP: Export to ppm, remove comment manually
# * Web: https://neuralcoder3.github.io/ppm-converter/   (Note: most converters found on Google return binary PPM files instead of ascii)

FILE=$1
if [ -z "$FILE" ]; then
    echo "Usage: $0 <image>"
    exit 1
fi
FORMAT=${FILE##*.}
echo "Converting $FILE to ppm"

if [ "$FORMAT" = "ppm" ]; then
    # format is ppm => check validity, strip comment
    if [ `head -n 1 $FILE` != "P3" ]; then
        echo "Error: $FILE is not a valid P3 ppm file"
        exit 1
    fi
    # strip comment (lines starting with #, can be multiple in the beginning)
    COPY=`mktemp`
    cp $FILE $COPY
    sed -i -e '/^#/d' $FILE
    # check if different
    if [ `diff $FILE $COPY | wc -l` -gt 0 ]; then
        echo "Comment stripped"
        # place the original file back in the folder with suffix _orig
        mv $COPY ${FILE%.*}_orig.$FORMAT
    else
        rm $COPY
    fi
    exit 0
fi

# if convert (ImageMagick) is not available and under arch ask the user to install it
if ! which convert &> /dev/null; then
    echo "Error: convert (ImageMagick) is not installed"
    echo "Should I install it for you? (y/n)"
    read -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo pacman -S imagemagick
    else 
        exit 1
    fi
fi

OUTFILE=${FILE%.*}.ppm
convert $FILE -compress none $OUTFILE

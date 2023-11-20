import os
from PIL import Image
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--indir', '-i', help='Input directory', type=str, required=True)
    parser.add_argument('--outdir', '-o', help='Output directory', type=str, required=True)
    args = parser.parse_args()

    if not os.path.exists(args.outdir):
        os.makedirs(args.outdir)

    for f in os.listdir(args.indir):
        in_file = os.path.join(args.indir, f)
        if not os.path.isfile(in_file):
            continue
        img = Image.open(os.path.join(args.indir, f))
        img = img.resize((1280, 720), resample=Image.Resampling.BICUBIC)
        filename, ext = os.path.splitext(f)
        img.save(os.path.join(args.outdir, filename + '.jpg'), format='jpeg', quality=80, optimize=True)
        print(f)

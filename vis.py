import Image, ImageDraw
import sys

if len(sys.argv) < 2:
	print 'usage: ./prog image offset_file'
	exit(0)
print sys.argv[1]
im = Image.open(sys.argv[1])
draw = ImageDraw.Draw(im)
f = open(sys.argv[2])

offsets = []
for l in f:
	t  = l.split()
	offsets.append((float(t[0]),float(t[1])))

for o in offsets:
	xy = (int(o[0]) , int(o[1]), int(o[0])+5 , int(o[1])+5)
	draw.ellipse(xy )

im.save('TESTING.png','png')


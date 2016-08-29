height=310;
width=310;

boxheight=10;
boxwidth=17;
//boxheight=1;
//boxwidth=1;

colwidth=28.3;
rowheight=31;

rows=10-1;
cols=11-1;

linewidth=cols*colwidth;
lineheight=rows*rowheight;

spaceH=(width-linewidth)/2;
spaceV=(height-lineheight)/2;

module row() {
  for (c = [0:cols]) {
    translate([colwidth*c,0]) square([boxwidth,boxheight], true);
  }
}

module grid() {
  for (r = [0:rows]) {
    translate([0,rowheight*r]) row(11);
  }
}

difference() {
    square([height,width]);
    translate([spaceH,spaceV]) grid();
    translate([84,93.4]) circle (r=8.5);
    translate([226,93.4]) circle (r=8.5);
    translate([84,216.6]) circle (r=8.5);
    translate([226,216.6]) circle (r=8.5);
};


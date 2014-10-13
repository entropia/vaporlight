edge_length = 25;
edge_stop_length = edge_length - 10;
wall_width = 2;
wall_height = 2;
slot_height = 3;

total_height = 2 * wall_height + slot_height;

round_radius = 3;


// Rotate everything so that the long side points down
rotate([90,135,0])

// Construct body
difference() {
    // Outer extruded triangle
    linear_extrude(height = total_height, center = true, convexity = 10, $fn=100) {
        polygon(points=[[0,0],[edge_length,0],[0,edge_length]], paths=[[0,1,2]]);
    }

    // Inner extruded triangle
    linear_extrude(height = slot_height, center = true, convexity = 10, $fn=100) {
        polygon(points=[[wall_width,wall_width],[edge_length,wall_width],[wall_width,edge_length]], paths=[[0,1,2]]);
    }

    // Round off 90deg corner
    difference(){
        cube([2*round_radius,2*round_radius,total_height], center = true);

        translate([round_radius, round_radius, 0])
            cylinder (h = total_height, r=round_radius, center = true, $fn=100);
    }

    // Cut off 45deg corners
    translate([edge_length, 0, 0])
        rotate([0,0,45])
            cube([edge_length - edge_stop_length, edge_length - edge_stop_length, edge_length - edge_stop_length], center=true);
    translate([0, edge_length, 0])
        rotate([0,0,45])
            cube([edge_length - edge_stop_length, edge_length - edge_stop_length, edge_length - edge_stop_length], center=true);
}


import sys
import xml.etree.ElementTree as ET
import random
from math import sin, cos, pi
from xml.dom import minidom

def random_coords(inner_radius, outer_radius):
    # Generate random spherical coordinates and convert to Cartesian
    yaw = random.uniform(0, 2 * pi)
    pitch = random.uniform(-pi/2, pi/2)
    r = random.uniform(inner_radius, outer_radius)
    
    x = r * cos(pitch) * sin(yaw)
    y = r * sin(pitch)
    z = r * cos(pitch) * cos(yaw)
    
    return x, y, z

def add_stars_to_xml(xml_content, num_stars=100, inner_radius=300, outer_radius=700, star_size=1):
    world_element = ET.fromstring(xml_content)
    stars_group = ET.Element('group', {'desc': 'STARS'})
    
    for i in range(num_stars):
        x, y, z = random_coords(inner_radius, outer_radius)
        
        star_subgroup = ET.Element('group', {'desc': f'STAR_{i+1}'})
        
        colour = ET.SubElement(star_subgroup, 'colour')
        colour.set('r', '1.0')
        colour.set('g', '1.0')
        colour.set('b', '1.0')
        
        transform = ET.SubElement(star_subgroup, 'transform')
        translate = ET.SubElement(transform, 'translate')
        translate.set('x', str(x))
        translate.set('y', str(y))
        translate.set('z', str(z))
        
        scale = ET.SubElement(transform, 'scale')
        scale.set('x', str(star_size))
        scale.set('y', str(star_size))
        scale.set('z', str(star_size))
        
        models = ET.SubElement(star_subgroup, 'models')
        model = ET.SubElement(models, 'model')
        model.set('file', 'sphere.3d')
        
        stars_group.append(star_subgroup)
    
    world_element.append(stars_group)
    
    return ET.tostring(world_element, encoding='unicode')

def prettify(xml_string):
    """Return a pretty-printed XML string for the Element."""
    parsed = minidom.parseString(xml_string)
    return parsed.toprettyxml(indent="\t")

if __name__ == "__main__":
    xml_content = sys.stdin.read()
    
    modified_xml = add_stars_to_xml(xml_content)
    
    # Remove the XML declaration that minidom adds (optional)
    pretty_xml = prettify(modified_xml)
    pretty_xml = '\n'.join(line for line in pretty_xml.split('\n') if line.strip())
    
    print(pretty_xml)

# powershell: Get-Content config.xml | python3 add_stars.py > config_with_stars.xml








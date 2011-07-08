#! /usr/bin/env python

class WorldTile(object):
    TILE_TEXT_WIDTH  = 6
    TILE_TEXT_HEIGHT = 5

    def __init__(self): pass

    def place_city(self, city): pass

class OceanTile(WorldTile):
    def __init__(self, depth, climate, geology): pass

class LandTile(WorldTile):
    def __init__(self, yield_, climate, geology): pass

class MountainTile(LandTile):
    def __init__(self, elevation, climate, geology): pass

class DesertTile(LandTile):
    def __init__(self, climate, geology): pass

class TundraTile(LandTile):
    def __init__(self, climate, geology): pass

class HillsTile(LandTile):
    def __init__(self, climate, geology): pass

class FoodTile(LandTile):
    def __init__(self, yield_, climate, geology): pass

class PlainsTile(FoodTile):
    def __init__(self, climate, geology): pass

class LushTile(FoodTile):
    def __init__(self, climate, geology): pass

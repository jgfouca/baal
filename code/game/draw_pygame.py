#! /usr/bin/env python

"""
API for drawing things in pygame.
"""

import unittest, pygame, os

from Tkinter import Tk, Label, Button, LEFT

from baal_common import prequire
from draw_mode import DrawMode, curr_draw_mode, _set_draw_mode
from baal_time import Time, Season
from geology import Geology, is_geological, \
    Divergent, Subducting, Orogenic, Transform, Inactive
from world_tile import WorldTile, Yield, \
    OceanTile, MountainTile, DesertTile, TundraTile, HillsTile, PlainsTile, \
    LushTile, get_flooding_threshold, get_totally_flooded_threshold
from player import Player
from player_ai import PlayerAI
from weather import Atmosphere, Anomaly, is_atmospheric
from world import World
from engine import engine

# TODO - This is prototype code and needs extensive cleanup

# Some RGB color constants

_RED        = (255, 0, 0)
_GREEN      = (0, 255, 0)
_BLUE       = (0, 0, 255)
_YELLOW     = (255, 255, 0)
_WHITE      = (255, 255, 255)
_GREY       = (155, 155, 155)
_BLACK      = (0, 0, 0)
_DARKGREEN  = (0, 125, 0)
_DARKYELLOW = (125, 125, 0)

###############################################################################
class DrawPygame(object):
###############################################################################
    """
    Draw objects using pygame graphics.
    """

    SCREEN_WIDTH  = 1200
    SCREEN_HEIGHT = 800

    #
    # ==== Public API ====
    #

    ###########################################################################
    def __init__(self):
    ###########################################################################
        self.__x_pos = 0
        self.__y_pos = 0

        # Initialize pygame
        pygame.init()
        self.__screen = pygame.display.set_mode((self.SCREEN_WIDTH,
                                                 self.SCREEN_HEIGHT))

        self._LARGE_FONT  = pygame.font.SysFont("None", # font name
                                                30)     # fontsize
        self._NORMAL_FONT = pygame.font.SysFont("None", # font name
                                                24)     # fontsize


        # Compute path to data area
        self.__path_to_data = os.path.abspath(
            os.path.join(os.path.dirname(__file__),
                         os.path.pardir, os.path.pardir, "data")
            )

        prequire(os.path.isdir(self.__path_to_data),
                 self.__path_to_data, " does not exist.")

    ###########################################################################
    def begin_draw(self):
    ###########################################################################
        self.__screen.fill( _BLACK )

        self.__x_pos = 0
        self.__y_pos = 0

        self.__cast_spell_button_loc_map  = {}
        self.__learn_spell_button_loc_map = {}
        self.__draw_mode_button_loc_map   = {}

        self.__spell_to_cast = None

        self.__next_button_pos = None
        self.__next_button_dim = None

        self.__hack_button_pos = None
        self.__hack_button_dim = None

    ###########################################################################
    def end_draw(self):
    ###########################################################################
        # Pygame uses double-buffering; make the buffer we just drew into the
        # active buffer.
        pygame.display.flip()

    ###########################################################################
    def popup(self, title, text):
    ###########################################################################
        root = Tk()
        root.title(title)
        Label(root,text=text, justify=LEFT).pack(pady=10)
        Button(root, text="OK", command=lambda: root.destroy()).pack()

        w = root.winfo_screenwidth()
        h = root.winfo_screenheight()
        rw = root.winfo_reqwidth()
        rh = root.winfo_reqheight()
        x = w/2 - rw/2
        y = h/2 - rh/2
        root.geometry("+%d+%d" % (x, y))

        root.mainloop()

    ###########################################################################
    def draw(self, item):
    ###########################################################################
        if (isinstance(item, Time)):
            self.__draw_time(item)
        elif (isinstance(item, Geology)):
            self.__draw_geology(item)
        elif (isinstance(item, Player)):
            self.__draw_player(item)
        elif (isinstance(item, PlayerAI)):
            self.__draw_player_ai(item)
        elif (isinstance(item, Atmosphere)):
            self.__draw_atmosphere(item)
        elif (isinstance(item, Anomaly)):
            self.__draw_anomaly(item)
        elif (isinstance(item, World)):
            self.__draw_world(item)
        elif (isinstance(item, WorldTile)):
            self.__draw_world_tile(item)
        else:
            prequire(False, "Class not drawable: ", item.__class__)

    ###########################################################################
    def clicked(self, mouse_pos, button1, button2, button3):
    ###########################################################################
        """
        Based on mouse click, generate command text
        """
        # First: Determine what action to take based on what button was clicked
        HELP, DO = range(2)
        if (button1 and not button2 and not button3):
            action = DO
        elif (not button1 and not button2 and button3):
            action = HELP
        else:
            # ignore
            return None

        # Next: Figure out what the user is clicking on so we can create the
        # appropriate command...

        # Case 1: User is clicking one of the buttons for spell learning/casting
        for button_collection in [self.__cast_spell_button_loc_map,
                                  self.__learn_spell_button_loc_map,
                                  self.__draw_mode_button_loc_map]:
            for spell_name, button_loc in button_collection.iteritems():
                if (_is_within(mouse_pos, button_loc, 50)):
                    # Match
                    if (button_collection is self.__learn_spell_button_loc_map):
                        rv = "learn %s" % spell_name
                        return "help %s" % rv if action == HELP else rv
                    elif (button_collection is self.__cast_spell_button_loc_map):
                        self.__spell_to_cast = spell_name
                        return "help cast %s" % spell_name if action == HELP else None
                    else:
                        rv = "draw %s" % spell_name
                        return "help %s" % rv if action == HELP else rv

        # Case 2: User has chosen a spell to cast and is clicking on a world
        # tile to complete the cast
        if (self.__spell_to_cast and action == DO):
            if (_is_within(mouse_pos, self.__tile_start_pos,
                           self.__world_dim[0] * 100, self.__world_dim[0] * 100)):
                x_diff = mouse_pos[0] - self.__tile_start_pos[0]
                y_diff = mouse_pos[1] - self.__tile_start_pos[1]
                row_loc = int(y_diff / 100)
                col_loc = int(x_diff / 100)

                # For now, just cast it at the max level possible
                player = engine().player()
                spell_level = player.spell_skill(self.__spell_to_cast)

                return "cast %s %d %d,%d" % \
                    (self.__spell_to_cast, spell_level, row_loc, col_loc)

        # Case 3: User is clicking end turn
        if (_is_within(mouse_pos, self.__next_button_pos,
                       self.__next_button_dim[0],
                       self.__next_button_dim[1])):
            return "end" if action == DO else "help end"

        # Case 4: User is clicking hack
        if (_is_within(mouse_pos, self.__hack_button_pos,
                       self.__hack_button_dim[0],
                       self.__hack_button_dim[1])):
            return "hack" if action == DO else "help hack"

        return None

    #
    # ==== Internal Methods ====
    #

    ###########################################################################
    def _print(self, text, color, position, font=None):
    ###########################################################################
        font = self._NORMAL_FONT if font is None else font
        self.__screen.blit(font.render(text,
                                       True, # antialias
                                       color),
                           position)

    ###########################################################################
    def _print_with_offset(self, text, color, offset=(0,0), font=None):
    ###########################################################################
        """
        Print at a position relative to the current x_pos, y_pos
        """
        return self._print(text,
                           color,
                          (self.__x_pos + offset[0], self.__y_pos + offset[1]),
                           font)

    ###########################################################################
    def _draw(self, dirname, filename, size, position):
    ###########################################################################
        image = pygame.image.load(os.path.join(self.__path_to_data,
                                               "images", dirname,
                                               filename))
        self.__screen.blit(image,
                           (position[0], position[1], size[0], size[1]))

    ###########################################################################
    def _draw_with_offset(self, dirname, filename, size, offset=(0,0)):
    ###########################################################################
        """
        Draw an image at a position relative to the current x_pos, y_pos
        """
        self._draw(dirname, filename, size,
                   (self.__x_pos + offset[0], self.__y_pos + offset[1]))

    ###########################################################################
    def __draw_time(self, item):
    ###########################################################################
        # Compute color
        if (item.season() == Season.WINTER):
            color = _BLUE
        elif (item.season() == Season.SPRING):
            color = _GREEN
        elif (item.season() == Season.SUMMER):
            color = _RED
        elif (item.season() == Season.FALL):
            color = _YELLOW
        else:
            prequire(False, "Unhandled season ", item.season())

        self._print_with_offset("%s, Year: %s" % (item.season(), item.year()),
                                color,
                                (200, 5),
                                font=self._LARGE_FONT)

        self.__y_pos += 30
        self.__x_pos = 0

    ###########################################################################
    def __draw_geology(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()
        prequire(is_geological(draw_mode), "Bad draw mode ", draw_mode)

        self._draw_with_offset("geology",
                               item.__class__.__name__.lower() + ".jpg",
                               (100, 100))

        # Figure out what string to print and what color it should be
        if (draw_mode == DrawMode.GEOLOGY):
            pass # already drawn
        elif (draw_mode in [DrawMode.TENSION, DrawMode.MAGMA]):
            value = item.tension() if draw_mode == DrawMode.TENSION \
                else item.magma()
            to_draw = ("%.3f" % value)

            if (value < .333):
                color = _GREEN
            elif (value < .666):
                color = _YELLOW
            else:
                color = _RED

            self._print_with_offset(to_draw, color, (30, 40))
        else:
            prequire(False, "Should not draw geology in mode: ", draw_mode)

    ###########################################################################
    def __draw_player(self, item):
    ###########################################################################
        self._draw_with_offset("player", "baal.jpg", (100, 100))

        self.__x_pos += 100

        self._print_with_offset("PLAYER STATS:", _WHITE)

        self.__y_pos += 20

        self._print_with_offset("  name: %s" % item.name(), _WHITE)

        self.__y_pos += 20

        self._print_with_offset("  level: %d" % item.level(), _GREEN)

        self.__y_pos += 20

        self._print_with_offset("  mana: %d / %d" % (item.mana(), item.max_mana()), _BLUE)

        self.__y_pos += 20

        self._print_with_offset("  exp: %d / %d" % (item.exp(), item.next_level_cost()), _YELLOW)

        self.__x_pos += 200
        self.__y_pos -= 80

        # Draw spell buttons based on player spells

        x_button_pos = self.SCREEN_WIDTH - 600
        y_button_pos = 0

        self._print("CAST:", _RED, (x_button_pos + 5, y_button_pos))

        self._print("LEARN:", _GREEN, (x_button_pos + 165, y_button_pos))

        self._print("DRAW MODES:", _WHITE, (x_button_pos + 275, y_button_pos))

        y_button_pos = 15

        for spell_name, spell_level in item.talents():
            self._draw("spell-icons",
                       "%s.jpg" % spell_name,
                       (50, 50),
                       (x_button_pos, y_button_pos))

            self.__cast_spell_button_loc_map[spell_name] = (x_button_pos, y_button_pos)
            y_button_pos += 50

        x_button_pos += 110
        y_button_pos = 15

        for spell_name, spell_level in item.learnable():
            self._draw("spell-icons",
                       "%s.jpg" % spell_name,
                       (50, 50),
                       (x_button_pos, y_button_pos))

            self.__learn_spell_button_loc_map[spell_name] = (x_button_pos, y_button_pos)
            y_button_pos += 50

        x_button_pos += 140
        y_button_pos = 15

        for draw_mode in DrawMode:
            draw_mode_str = str(draw_mode).lower()
            self._draw("draw-modes",
                       "%s.jpg" % draw_mode_str,
                       (50, 50),
                       (x_button_pos, y_button_pos))

            self.__draw_mode_button_loc_map[draw_mode_str] = (x_button_pos, y_button_pos)
            y_button_pos += 50

    ###########################################################################
    def __draw_player_ai(self, item):
    ###########################################################################
        self._draw_with_offset("player", "peasant.png", (100, 100))

        orig_x_pos = self.__x_pos
        self.__x_pos += 100

        self._print_with_offset("AI PLAYER STATS:", _WHITE)
        self.__y_pos += 20

        self._print_with_offset("  tech level: %d" % item.tech_level(), _GREEN)
        self.__y_pos += 20

        self._print_with_offset("  population: %d" % item.population(), _BLUE)

        self.__x_pos = 0
        self.__y_pos += 75

        # We choose to draw this here since we have extra room
        self._draw("misc", "next_turn.jpg", (100, 50), (orig_x_pos + 50, self.__y_pos))

        self.__next_button_pos = (orig_x_pos + 50, self.__y_pos)
        self.__next_button_dim = (100, 50)

        self._draw("misc", "hack.jpg", (100, 50), (orig_x_pos + 175, self.__y_pos))

        self.__hack_button_pos = (orig_x_pos + 175, self.__y_pos)
        self.__hack_button_dim = (100, 50)

        self.__y_pos += 50


    # Describes how to draw the various fields. The first value in the pair
    # is the upper-bound for the corresponding color.
    _MAX = 999999999
    _ATMOS_COLOR_MAP = {
        DrawMode.WIND        : ((10,  _DARKGREEN), (20, _YELLOW), (_MAX, _RED)),
        DrawMode.DEWPOINT    : ((32,  _RED), (55, _YELLOW), (_MAX, _DARKGREEN)),
        DrawMode.TEMPERATURE : ((32,  _BLUE), (80, _YELLOW), (_MAX, _RED)),
        DrawMode.PRESSURE    : ((975, _DARKGREEN), (1025, _YELLOW), (_MAX, _RED)),
        DrawMode.PRECIP      : ((2,   _RED), (10, _YELLOW), (_MAX, _GREEN))
    }

    ###########################################################################
    def __compute_color(self, draw_mode, field_value, color_map):
    ###########################################################################
        for upper_bound, color in color_map[draw_mode]:
            if (field_value < upper_bound):
                return color

        prequire(False,
                 "Failed to find color for ", draw_mode, ", val ", field_value)

    ###########################################################################
    def __get_field_for_draw_mode(self, item, draw_mode):
    ###########################################################################
        if (draw_mode == DrawMode.WIND):
            return item.wind().speed()
        elif (draw_mode == DrawMode.DEWPOINT):
            return item.dewpoint()
        elif (draw_mode == DrawMode.TEMPERATURE):
            return item.temperature()
        elif (draw_mode == DrawMode.PRESSURE):
            return item.pressure()
        elif (draw_mode == DrawMode.PRECIP):
            return item.precip()
        else:
            prequire(False, "Bad draw mode: ", draw_mode)

    ###########################################################################
    def __draw_atmosphere(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()

        field = self.__get_field_for_draw_mode(item, draw_mode)
        color = self.__compute_color(draw_mode, field, self._ATMOS_COLOR_MAP)

        to_draw = ("%.2f" % field)

        self._print_with_offset(to_draw, color, (30, 40))

    ###########################################################################
    def __draw_anomaly(self, item):
    ###########################################################################
        # TODO
        # print("Level:", item.intensity(), item.category(),
        #       "anomaly at location", item.location(),
        #       end="")
        pass

    ###########################################################################
    def __draw_world(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()

        self.draw(item.time())

        self.__world_dim = (item.width(), item.height())

        self.__tile_start_pos = (self.__x_pos, self.__y_pos)

        # Draw tiles
        for row in item.iter_rows():
            for tile in row:
                self.__draw_world_tile(tile)
            self.__x_pos = 0
            self.__y_pos += 100

        # TODO
        # Draw recent anomalies
        # for anomaly in item.iter_anomalies():
        #     self.draw(anomaly)
        #     print()

    _TILE_MAP = {
        OceanTile    : "ocean1.jpg",
        MountainTile : "mountain3.jpg",
        DesertTile   : "desert1.jpg",
        TundraTile   : "tundra1.jpg",
        HillsTile    : "hills1.jpg",
        PlainsTile   : "plains3.jpg",
        LushTile     : "grass1.jpg"
    }

    ###########################################################################
    def __draw_land(self, item):
    ###########################################################################
        filename = self._TILE_MAP[item.__class__]
        self._draw_with_offset("tiles", filename, (100, 100))

    _LAND_COLOR_MAP = {
        DrawMode.MOISTURE       : ((1.0, _YELLOW),
                                   (get_flooding_threshold(), _DARKGREEN),
                                   (get_totally_flooded_threshold(), _BLUE),
                                   (_MAX, _RED)),
        DrawMode.YIELD          : ((Yield(_MAX, 0), _DARKGREEN),
                                   (Yield(0, _MAX), _RED)),
        DrawMode.ELEVATION      : ((2500, _DARKGREEN),
                                   (7000, _YELLOW),
                                   (_MAX, _GREY)),
        DrawMode.SNOWPACK       : ((12, _YELLOW), (40, _BLUE), (_MAX, _GREY)),
        DrawMode.SEASURFACETEMP : ((65, _BLUE), (80, _YELLOW), (_MAX, _RED))
    }

    ###########################################################################
    def __determine_precision(self, field):
    ###########################################################################
        if (abs(field) > 1000):
            return "%d" % field
        elif (abs(field) > 10):
            return "%.2f" % field
        else:
            return "%.3f" % field

    ###########################################################################
    def __draw_land_property(self, draw_mode, tile):
    ###########################################################################
        self.__draw_land(tile)

        if (draw_mode == DrawMode.MOISTURE):
            field = tile.soil_moisture()
        elif (draw_mode == DrawMode.YIELD):
            field = tile.yield_()
        elif (draw_mode == DrawMode.ELEVATION):
            field = tile.elevation()
        elif (draw_mode == DrawMode.SNOWPACK):
            field = tile.snowpack()
        elif (draw_mode == DrawMode.SEASURFACETEMP):
            field = tile.sea_surface_temp()
        else:
            prequire(False, "Unknown draw mode ", draw_mode)

        if (field is not None):
            color   = self.__compute_color(draw_mode, field, self._LAND_COLOR_MAP)
            to_draw = self.__determine_precision(field)

            self._print_with_offset(to_draw, color, (30, 40))

    ###########################################################################
    def __draw_world_tile(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()

        if (draw_mode == DrawMode.LAND):
            self.__draw_land(item)

        elif (draw_mode == DrawMode.CIV):
            self.__draw_land(item)
            if (item.city() is not None):
                self._draw_with_offset("misc", "city.jpg", (50, 50), (25, 25))

                self._print_with_offset("Size: %d" % item.city().rank(), _RED, (30, 75))

            elif (item.infra_level() is not None and item.infra_level() > 0):
                image = "farm.jpg" if (item.yield_().food > 0) else "mine.jpg"

                self._draw_with_offset("misc", image, (50, 50), (25, 25))

                self._print_with_offset("Level: %d" % item.infra_level(), _YELLOW, (30, 75))

        elif (is_geological(draw_mode)):
            self.draw(item.geology())

        elif (is_atmospheric(draw_mode)):
            self.__draw_land(item)
            self.draw(item.atmosphere())

        else:
            self.__draw_land_property(draw_mode, item)

        self.__x_pos += 100

#
# Free functions
#

###############################################################################
def _is_within( position, square_begin, square_size_x, square_size_y=None):
###############################################################################
    if (square_size_y is None):
        square_size_y = square_size_x
    return (position[0] >= square_begin[0]                 and
            position[0] <  square_begin[0] + square_size_x and
            position[1] >= square_begin[1]                 and
            position[1] <  square_begin[1] + square_size_y)

#
# Tests
#

###############################################################################
class TestDrawPygame(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_draw_text(self):
    ###########################################################################
        # TODO
        pass

if (__name__ == "__main__"):
    unittest.main()

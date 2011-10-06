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
from world_tile import WorldTile, \
    OceanTile, MountainTile, DesertTile, TundraTile, HillsTile, PlainsTile, \
    LushTile, FoodTile
from player import Player
from player_ai import PlayerAI
from weather import Atmosphere, Anomaly, is_atmospheric
from world import World
from engine import engine

# TODO - This is prototype code and needs extensive cleanup

# Some RGB color constants

_RED       = (255, 0, 0)
_GREEN     = (0, 255, 0)
_BLUE      = (0, 0, 255)
_YELLOW    = (255, 255, 0)
_WHITE     = (255, 255, 255)
_BLACK     = (0, 0, 0)
_DARKGREEN = (0, 125, 0)

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

        return None

    #
    # ==== Internal Methods ====
    #

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

        my_font = pygame.font.SysFont("None", # font name
                                      30)     # fontsize
        self.__screen.blit(my_font.render("%s, Year: %s" % (item.season(), item.year()),
                                          0, # antialias
                                          color),
                           (self.__x_pos + 200, self.__y_pos + 5))

        self.__y_pos += 30
        self.__x_pos = 0

    ###########################################################################
    def __draw_geology(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()
        prequire(is_geological(draw_mode), "Bad draw mode ", draw_mode)

        geological_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                          "images", "geology",
                                                          item.__class__.__name__.lower() + ".jpg"))
        self.__screen.blit(geological_image,
                           (self.__x_pos, self.__y_pos, 100, 100))

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

            my_font = pygame.font.SysFont("None", # font name
                                          20)     # fontsize

            self.__screen.blit(my_font.render(to_draw,
                                              0, # antialias
                                              color),
                               (self.__x_pos + 30, self.__y_pos + 40))
        else:
            prequire(False, "Should not draw geology in mode: ", draw_mode)

    ###########################################################################
    def __draw_player(self, item):
    ###########################################################################
        player_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                      "images", "player",
                                                      "baal.jpg"))
        self.__screen.blit(player_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels

        self.__x_pos += 100

        my_font = pygame.font.SysFont("None", # font name
                                      20)     # fontsize

        self.__screen.blit(my_font.render("PLAYER STATS:",
                                   0, # antialias
                                   _WHITE),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  name: %s" % item.name(),
                                   0, # antialias
                                   _WHITE),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  level: %d" % item.level(),
                                   0, # antialias
                                   _GREEN),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  mana: %d / %d" % (item.mana(), item.max_mana()),
                                   0, # antialias
                                   _BLUE),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  exp: %d / %d" % (item.exp(), item.next_level_cost()),
                                          0, # antialias
                                          _YELLOW),
                           (self.__x_pos, self.__y_pos))

        self.__x_pos += 200
        self.__y_pos -= 80

        # Draw spell buttons based on player spells

        x_button_pos = self.SCREEN_WIDTH - 600
        y_button_pos = 0

        self.__screen.blit(my_font.render("CAST:",
                                          0, # antialias
                                          _RED),
                           (x_button_pos + 5, y_button_pos))

        self.__screen.blit(my_font.render("LEARN:",
                                          0, # antialias
                                          _GREEN),
                           (x_button_pos + 165, y_button_pos))

        self.__screen.blit(my_font.render("DRAW MODES:",
                                          0, # antialias
                                          _WHITE),
                           (x_button_pos + 275, y_button_pos))

        y_button_pos = 15

        for spell_name, spell_level in item.talents():
            spell_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                         "images", "spell-icons",
                                                         "%s.jpg" % spell_name))
            self.__screen.blit(spell_image,
                               (x_button_pos, y_button_pos, 50, 50))
            self.__cast_spell_button_loc_map[spell_name] = (x_button_pos, y_button_pos)
            y_button_pos += 50

        x_button_pos += 110
        y_button_pos = 15

        for spell_name, spell_level in item.learnable():
            spell_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                         "images", "spell-icons",
                                                         "%s.jpg" % spell_name))
            self.__screen.blit(spell_image,
                               (x_button_pos, y_button_pos, 50, 50))
            self.__learn_spell_button_loc_map[spell_name] = (x_button_pos, y_button_pos)
            y_button_pos += 50

        x_button_pos += 140
        y_button_pos = 15

        for draw_mode in DrawMode:
            draw_mode_str = str(draw_mode).lower()
            mode_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                         "images", "draw-modes",
                                                         "%s.jpg" % draw_mode_str))
            self.__screen.blit(mode_image,
                               (x_button_pos, y_button_pos, 50, 50))
            self.__draw_mode_button_loc_map[draw_mode_str] = (x_button_pos, y_button_pos)
            y_button_pos += 50

    ###########################################################################
    def __draw_player_ai(self, item):
    ###########################################################################
        player_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                      "images", "player",
                                                      "peasant.png"))
        self.__screen.blit(player_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels

        orig_x_pos = self.__x_pos
        self.__x_pos += 100

        my_font = pygame.font.SysFont("None", # font name
                                      20)     # fontsize

        self.__screen.blit(my_font.render("AI PLAYER STATS:",
                                          0, # antialias
                                          _WHITE),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  tech level: %d" % item.tech_level(),
                                          0, # antialias
                                          _GREEN),
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  population: %d" % item.population(),
                                          0, # antialias
                                          _BLUE),
                    (self.__x_pos, self.__y_pos))

        self.__x_pos = 0
        self.__y_pos += 75

        # We choose to draw this here since we have extra room
        next_button_img = pygame.image.load(os.path.join(self.__path_to_data,
                                                         "images", "misc",
                                                         "next_turn.jpg"))

        self.__screen.blit(next_button_img,
                           (orig_x_pos + 50, self.__y_pos, 100, 50))
        self.__next_button_pos = (orig_x_pos + 50, self.__y_pos)
        self.__next_button_dim = (100, 50)

        self.__y_pos += 50


    # Describes how to draw the various fields. The first value in the pair
    # is the upper-bound for the corresponding color.
    _MAX = 999999999
    _ATMOS_FIELD_COLOR_MAP = {
        DrawMode.WIND        : ((10,  _DARKGREEN), (20, _YELLOW), (_MAX, _RED)),
        DrawMode.DEWPOINT    : ((32,  _RED), (55, _YELLOW), (_MAX, _DARKGREEN)),
        DrawMode.TEMPERATURE : ((32,  _BLUE), (80, _YELLOW), (_MAX, _RED)),
        DrawMode.PRESSURE    : ((975, _DARKGREEN), (1025, _YELLOW), (_MAX, _RED)),
        DrawMode.RAINFALL    : ((2,   _RED), (10, _YELLOW), (_MAX, _RED))
    }

    ###########################################################################
    def __compute_atmos_color(self, draw_mode, field_value):
    ###########################################################################
        for upper_bound, color in self._ATMOS_FIELD_COLOR_MAP[draw_mode]:
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
        elif (draw_mode == DrawMode.RAINFALL):
            return item.precip()
        else:
            prequire(False, "Bad draw mode: ", draw_mode)

    ###########################################################################
    def __draw_atmosphere(self, item):
    ###########################################################################
        draw_mode = curr_draw_mode()

        field = self.__get_field_for_draw_mode(item, draw_mode)
        color = self.__compute_atmos_color(draw_mode, field)

        to_draw = ("%.2f" % field)

        my_font = pygame.font.SysFont("None", # font name
                                      20)     # fontsize

        self.__screen.blit(my_font.render(to_draw,
                                          0, # antialias
                                          color),
                           (self.__x_pos + 30, self.__y_pos + 40))

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
        tile_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                    "images", "tiles",
                                                    filename))
        self.__screen.blit(tile_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels

    ###########################################################################
    def __draw_world_tile(self, item):
    ###########################################################################
        my_font = pygame.font.SysFont("None", # font name
                                      20)     # fontsize

        draw_mode = curr_draw_mode()

        if (draw_mode == DrawMode.LAND):
            self.__draw_land(item)

        elif (draw_mode == DrawMode.CIV):
            self.__draw_land(item)
            if (item.city() is not None):
                city_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                            "images", "misc",
                                                            "city.jpg"))
                self.__screen.blit(city_image,
                                   (self.__x_pos + 25,
                                    self.__y_pos + 25,
                                    50, 50))

                self.__screen.blit(my_font.render("Size: %d" % item.city().rank(),
                                                  0, # antialias
                                                  _RED),
                                   (self.__x_pos + 30, self.__y_pos + 75))

            elif (item.infra_level() is not None and item.infra_level() > 0):
                if (isinstance(item, FoodTile)):
                    infra_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                                 "images", "misc",
                                                                 "farm.jpg"))
                else:
                    infra_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                                 "images", "misc",
                                                                 "mine.jpg"))

                self.__screen.blit(infra_image,
                                   (self.__x_pos + 25,
                                    self.__y_pos + 25,
                                    50, 50))

                self.__screen.blit(my_font.render("Level: %d" % item.infra_level(),
                                                  0, # antialias
                                                  _YELLOW),
                                   (self.__x_pos + 30, self.__y_pos + 75))

        elif (draw_mode == DrawMode.MOISTURE):
            self.__draw_land(item)
            moisture = item.soil_moisture()
            if (moisture is not None):
                if (moisture < 1.0):
                    color = _YELLOW
                elif (moisture < FoodTile.FLOODING_THRESHOLD):
                    color = _DARKGREEN
                elif (moisture < FoodTile.TOTALLY_FLOODED):
                    color = _BLUE
                else:
                    color = _RED

                to_draw = ("%.3f" % moisture)

                my_font = pygame.font.SysFont("None", # font name
                                              20)     # fontsize

                self.__screen.blit(my_font.render(to_draw,
                                                  0, # antialias
                                                  color),
                                   (self.__x_pos + 30, self.__y_pos + 40))

        elif (draw_mode == DrawMode.YIELD):
            self.__draw_land(item)
            yield_ = item.yield_()
            if (yield_.food > 0):
                color = _DARKGREEN
                to_draw = "%.3f" % yield_.food
            else:
                color = _RED
                to_draw = "%.3f" % yield_.prod

            my_font = pygame.font.SysFont("None", # font name
                                          20)     # fontsize

            self.__screen.blit(my_font.render(to_draw,
                                              0, # antialias
                                              color),
                               (self.__x_pos + 30, self.__y_pos + 40))

        elif (is_geological(draw_mode)):
            self.draw(item.geology())

        elif (is_atmospheric(draw_mode)):
            self.__draw_land(item)
            self.draw(item.atmosphere())

        else:
            prequire(False, "Unhandled mode: ", draw_mode)

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

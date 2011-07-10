#! /usr/bin/env python

from baal_common import prequire, Location, check_access, grant_access
from engine import engine

###############################################################################
class City(object):
###############################################################################
    """
    Represents human-built cities. TODO: describe conceptually how cities work.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, name, location): self.__init_impl(name, location)

    #
    # Query / Getter API
    #

    def name(self): return self.__name

    def population(self): return self.__population

    def rank(self): return self.__rank

    def location(self): return self.__location

    def famine(self): return self.__famine

    def defense(self): return self.__defense

    def to_xml(self): return self.__to_xml_impl()

    #
    # Modification API
    #

    def cycle_turn(self):
        """
        Tell this city that the turn is cycling
        """
        return self.__cycle_turn_impl()

    def kill(self, killed):
        """
        Kill off some of this city's citizens
        """
        return self.__kill_impl(killed)

    #
    # ==== Class constants ====
    #

    # City growth/food/production constants
    _CITY_BASE_GROWTH_RATE = 0.01 # 1% per turn
    _MAX_GROWTH_MODIFIER   = 4.0
    _CITY_RANK_UP_MULTIPLIER = 2
    _CITY_STARTING_POP = 1000
    MIN_CITY_SIZE = _CITY_STARTING_POP / 5
    _POP_THAT_EATS_ONE_FOOD = 1000
    _FOOD_FROM_CITY_CENTER = 1.0
    _PROD_FROM_CITY_CENTER = 1.0
    _PROD_FROM_SPECIALIST  = 1.0

    # Constants for production-costs of various buildable items
    _SETTLER_PROD_COST  = 200
    _INFRA_PROD_COST    = 50
    _CITY_DEF_PROD_COST = 400

    # AI constants
    _TOO_MANY_FOOD_WORKERS = 0.66
    _PROD_BEFORE_SETTLER   = 7.0

    # Access-limiting vars
    ALLOW_KILL       = "_allow_city_kill"
    ALLOW_CYCLE_TURN = "_allow_city_cycle"

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, name, location):
    ###########################################################################
        self.__name = name
        self.__rank = 1
        self.__population = City._CITY_STARTING_POP
        self.__next_rank_pop = self.__population * City._CITY_RANK_UP_MULTIPLIER
        self.__prod_bank = 0.0
        self.__location = location
        self.__defense = 1
        self.__famine = False

        from world_tile import WorldTile, LandTile
        from world import World

        grant_access(self, WorldTile.ALLOW_WORK)
        grant_access(self, LandTile.ALLOW_BUILD_INFRA)
        grant_access(self, World.ALLOW_PLACE_CITY)

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        # TODO: This method is still too big, break it up more

        check_access(self.__class__.ALLOW_CYCLE_TURN)

        prequire(self.__population > 0,
                 "This city has no people and should have been deleted")

        world  = engine().world()
        my_row, my_col = self.__location.unpack()

        # Gather resources based on nearby worked tiles. At this time,
        # cities will only be able to harvest adjacent tiles.

        # Evaluate nearby tiles, put in to sorted lists
        # (best-to-worst) for each of the two yield types
        food_tiles, prod_tiles = \
            _compute_nearby_food_and_prod_tiles(self.__location)

        # Choose which tiles to work. We first make sure we have ample
        # food, then we look for production tiles.

        req_food = self.__population / City._POP_THAT_EATS_ONE_FOOD
        food_gathered = City._FOOD_FROM_CITY_CENTER
        prod_gathered = City._PROD_FROM_CITY_CENTER
        num_workers = self.__rank
        num_workers_used_for_food = 0
        for tile in food_tiles:
            if (num_workers == 0): break

            if (food_gathered < req_food):
                tile.work()
                num_workers -= 1
                num_workers_used_for_food += 1
                food_gathered += tile.yield_().food
            else:
                # TODO: AI should continue to pile up food to increase
                # it's growth rate unless it's already hit the max
                # growth rate or if there are good production tiles
                # available or it really needs production.
                break

        # TODO: If city is getting close to being food capped, do
        # not sacrafice good production tiles for marginal food
        # tiles.
        for tile in prod_tiles:
            if (num_workers == 0): break

            if (tile.yield_().prod > City._PROD_FROM_SPECIALIST):
                tile.work()
                num_workers -= 1
                prod_gathered += tile.yield_().prod

        # Remaining workers are specialists that contribute production
        base_specialist_prod = num_workers * City._PROD_FROM_SPECIALIST
        prod_gathered += \
            engine().ai_player().get_adjusted_yield(base_specialist_prod)

        # Accumulate production
        self.__prod_bank += prod_gathered

        # Decide on how to spend production. Options: City
        # fortifications, tile infrastructure, or settler. In the
        # current system, the AI builds general production points and
        # it can use them to insta-buy the things it wants. This is a
        # bit unrealistic and should probably be replaced with a
        # system where the AI chooses to start building something and
        # all production points go to that thing until it is finished
        # (a la civ).

        # First, we need to decide what production item we should be
        # saving up for. Note that the AI will *not* build a cheaper
        # item that is of lower priority if it cannot afford the
        # highest priority item. It will always save-up for the
        # highest priority.

        # We want some of our workers doing something other than just
        # collecting food. If we are having to dedicate our workforce
        # to food, we probably need better food infrastructure. We
        # need to verify that there are nearby food tiles that we can
        # enhance.
        food_tile = None

        pct_workers_on_food = num_workers_used_for_food / self.__rank
        if (pct_workers_on_food > City._TOO_MANY_FOOD_WORKERS or
            food_gathered < req_food):
            for tile in food_tiles:
                if (tile.can_build_infra()):
                    food_tile = tile
                    break

        # We want a healthy level of production from this city if
        # possible. We need to verify that there are nearby production
        # tiles that we can enhance.
        prod_tile = None

        if (prod_gathered < City._PROD_BEFORE_SETTLER):
            for tile in prod_tiles:
                if (tile.can_build_infra()):
                    prod_tile = tile
                    break

        # We want to expand with settlers once a city has become large enough.
        # Check if building a settler is appropriate. New cities must be
        # "adjacent" to the city that created the settler.
        settler_loc = None
        max_distance = 3
        min_distance = 2
        heuristic_of_best_loc_so_far = 0.0
        for row_delta in xrange(-max_distance, max_distance + 1):
            for col_delta in xrange(-max_distance, max_distance + 1):
                loc = Location(my_row + row_delta, my_col + col_delta)

                # Check if this is a valid city loc
                if (world.in_bounds(loc) and
                    world.tile(loc).supports_city() and
                    not _is_too_close_to_any_city(loc, min_distance - 1)):

                    heuristic = _compute_city_loc_heuristic(loc)
                    if (heuristic > heuristic_of_best_loc_so_far):
                        settler_loc = loc
                        heuristic_of_best_loc_so_far = heuristic

        # We want a high level of production from this city if
        # possible. We need to verify that there are nearby production
        # tiles that we can enhance.
        prod_tile_fallback = None
        for tile in prod_tiles:
            if (tile.can_build_infra()):
                prod_tile_fallback = tile
                break

        # Now we actually try to build stuff. The order in which we
        # check the bools defines the priorities of the items.
        if (food_tile):
            self.__build_infra(food_tile)
        elif (prod_tile):
            self.__build_infra(prod_tile)
        elif (settler_loc):
            if (self.__prod_bank >= City._SETTLER_PROD_COST):
                world.place_city(settler_loc)
                self.__prod_bank -= City._SETTLER_PROD_COST
        elif (prod_tile_fallback):
            self.__build_infra(prod_tile_fallback)
        else:
            # No settler expansion is possible, build city
            # defenses. This is the lowest priority item to build.
            cost = self.__defense * City._CITY_DEF_PROD_COST
            if (self.__prod_bank >= cost):
                self.__defense += 1
                self.__prod_bank -= cost

        # Handle population growth

        # Compute multiplier, cannot exceed base by factor of > +-max-growth
        if (food_gathered < req_food):
            self.__famine = True
            food_multiplier = -req_food / food_gathered
            if (food_multiplier < -1 * City._MAX_GROWTH_MODIFIER):
                food_multiplier = -1 * City._MAX_GROWTH_MODIFIER
        else:
            self.__famine = False
            food_multiplier = food_gathered / req_food
            if (food_multiplier > City._MAX_GROWTH_MODIFIER):
                food_multiplier = City._MAX_GROWTH_MODIFIER

        # Grow city
        pop_growth_rate = 1 + (food_multiplier * City._CITY_BASE_GROWTH_RATE)
        self.__population *= pop_growth_rate
        if (self.__population > self.__next_rank_pop):
            self.__rank += 1
            self.__next_rank_pop *= City._CITY_RANK_UP_MULTIPLIER

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        return ""

    ###########################################################################
    def __kill_impl(self, killed):
    ###########################################################################
        check_access(self.__class__.ALLOW_KILL)

        prequire(self.__population >= killed, "Invalid killed: ", killed)

        self.__population -= killed

        if (self.__population > 0):
            while (self.__population <
                   self.__next_rank_pop / City._CITY_RANK_UP_MULTIPLIER):
                self.__rank -= 1
                self.__next_rank_pop /= City._CITY_RANK_UP_MULTIPLIER

    ###########################################################################
    def __build_infra(self, tile):
    ###########################################################################
        prequire(tile.can_build_infra(), "Error in build eval")

        infra_level = tile.infra_level()
        next_infra_level = infra_level + 1
        prod_cost = next_infra_level * City._INFRA_PROD_COST
        if (prod_cost < self.__prod_bank):
            self.__prod_bank -= prod_cost
            tile.build_infra()
            return True

        return False

#
# Internal functions
#

###############################################################################
def _ordered_insert(tiles, tile):
###############################################################################
    for idx, curr_tile in enumerate(tiles):
        curr_tile_food = curr_tile.yield_().food
        curr_tile_prod = curr_tile.yield_().prod
        new_tile_food  = tile.yield_().food
        new_tile_prod  = tile.yield_().prod

        if (curr_tile_food > 0):
            prequire(new_tile_food > 0, "Mismatch between list and new tile")
            if (new_tile_food > curr_tile_food):
                tiles.insert(idx, tile) # OK since iteration is over
                return
        else:
            prequire(curr_tile_prod > 0, "Curr tile is neither food nor prod?")
            prequire(new_tile_prod > 0, "Mismatch between list and new tile")
            if (new_tile_prod > curr_tile_prod):
                tiles.insert(idx, tile) # OK since iteration is over
                return

    tiles.append(tile)

###############################################################################
def _is_too_close_to_any_city(location, distance):
###############################################################################
    for city in engine().world().cities():
        row, col = location.unpack()
        city_row, city_col = city.location().unpack()
        if (abs(row - city_row) <= distance and
            abs(col - city_col) <= distance):
            return True

    return False

###############################################################################
def _compute_city_loc_heuristic(location):
###############################################################################
    food_tiles, prod_tiles = \
        _compute_nearby_food_and_prod_tiles(location,
                                            filter_tiles_near_other_cities=True)

    available_food = sum([tile.yield_().food for tile in food_tiles]) + \
        City._FOOD_FROM_CITY_CENTER
    available_prod = sum([tile.yield_().prod for tile in prod_tiles]) + \
        City._PROD_FROM_CITY_CENTER

    # Favor city locations with a good balance of food and production
    return available_food * available_prod

###############################################################################
def _compute_nearby_food_and_prod_tiles(location,
                                        filter_tiles_near_other_cities=False):
###############################################################################
    """
    Returns a 2-ple of (food-tiles, production-tiles); both lists are sorted
    from highest to lowest based on yield. Only unworked tiles are added.
    """
    world = engine().world()
    food_tiles = [] # sorted highest to lowest
    prod_tiles = [] # sorted highest to lowest
    row, col = location.unpack()
    for row_delta in xrange(-1, 2):
        for col_delta in xrange(-1, 2):
            # Cannot work tile that has city
            if (row_delta != 0 or col_delta != 0):
                loc_delta = Location(row + row_delta, col + col_delta)
                if (world.in_bounds(loc_delta) and
                    not (filter_tiles_near_other_cities and
                         _is_too_close_to_any_city(loc_delta, 1))):
                    tile = world.tile(loc_delta)
                    prequire(not (filter_tiles_near_other_cities and tile.worked()),
                             "How can tile be worked if it's not near a city")
                    if (not tile.worked()):
                        if (tile.yield_().food > 0):
                            _ordered_insert(food_tiles, tile)
                        else:
                            _ordered_insert(prod_tiles, tile)

    return food_tiles, prod_tiles

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()

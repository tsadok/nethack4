" NetHack4 Levelgen vim highlighting script
" Last modified by Sean Hunt, 2014-08-25
" Copyright (c) Sean Hunt, 2014
" This script may be freely redistributed under the same license as NetHack.
" See license for details.

let s:declarations = [
  \ "INIT_MAZE",
  \ "MAP", "SUBMAP",
  \ "REGION",
  \ "REGION_ARRAY", "COORD_ARRAY", "CHAR_ARRAY",
  \ "RETURN_LEV", "REPEAT"
\]

let s:statements = [
  \ "FILL_MAP", "MAKE_MAP",
  \ "NOTELEPORT", "SHORTSIGHTED", "NOMMAP", "HARDFLOOR", "ARBOREAL",
  \ "MAZEWALK", "PLACE_JUSTIFIED", "PLACE_AT",
  \ "TRAP", "OBJ", "OBJ_EX", "MON", "MON_EX", "OBJ_EX",
  \ "FILL_IRREGULAR", "LIGHT_REGION", "DARKEN_REGION", "NON_DIGGABLE",
  \ "TELEPORT_REGION", "STAIR_UP", "STAIR_DOWN", "IF_BRANCH_UP", "BRANCH_UP",
  \ "FOR_EACH_COORD", "FOR_EACH_REGION",
  \ "PLACE_DOOR", "PLACE_FOUNTAIN", "PLACE_ALTAR",
  \ "MAKE_ROOM", "MAKE_TEMPLE", "MAKE_MORGUE", "MAKE_ABANDONED_SHOP",
  \ "MAKE_SWAMP",
\]

let s:short_functions = [
  \ "C", "REL", "R", "REL_REG", "IN", "MR"
\]

let s:functions = [
  \ "RANDOM_MON_OF", "RANDOM_OBJ_OF",
  \ "MIMIC", "CORPSENM",
\]

let s:constants = [
  \ "WHOLEMAP",
  \ "UNALIGNED", "LAWFUL", "NEUTRAL", "CHAOTIC",
  \ "OPEN", "CLOSED", "LOCKED", "GONE", "TRAPPED", "BROKEN",
  \ "RANDOM_LOC", "RANDOM_OBJ", "RANDOM_MON",
  \ "SMOOTHED", "UNSMOOTHED", "JOINED", "UNJOINED",
  \ "LIT", "UNLIT", "WALLED", "UNWALLED",
  \ "UP", "DOWN", "BOTH",
  \ "LEFT", "RIGHT", "TOP", "BOTTOM", "CENTER", "HALF_LEFT",
  \ "NORTH", "SOUTH", "EAST", "WEST",
  \]

function! <SID>addmatches()
  if exists("b:matches") && !empty(b:matches) 
    return
  endif
  let b:matches = [
    \ matchadd("Special", '\<\(' . join(s:declarations, '\|') . '\)\>', -1),
    \ matchadd("Statement", '\<\(' . join(s:statements, '\|') . '\)\>', -1),
    \ matchadd("Macro", '\<\(' . join(s:short_functions, '\|') . '\)\>', -1),
    \ matchadd("Function", '\<\(' . join(s:functions, '\|') . '\)\>', -1),
    \ matchadd("Structure", '\<\(' . join(s:constants, '\|') . '\)\>', -1),
  \]
endf

function! <SID>delmatches()
  for l:match in b:matches
    call matchdelete(l:match)
  endfor

  let b:matches = []
endf

function! <SID>detect_highlights()
  if getline(3) =~ "nh4-scripts.*LEVGEN"
    EnableLevgenHighlights
  endif
endf

function! <SID>enable_highlights()
  call <SID>addmatches()
  augroup LevgenHighlights
    au! LevgenHighlights * <buffer>
    au BufWinLeave <buffer> call <SID>delmatches()
    au BufWinEnter <buffer> call <SID>addmatches()
  augroup END
endf

function! <SID>disable_highlights()
  call <SID>delmatches()
  au! LevgenHighlights * <buffer>
endf

command! EnableLevgenHighlights call <SID>enable_highlights()

command! DisableLevgenHighlights call <SID>disable_highlights()

augroup LevgenHighlights
  au!
  au BufReadPost * call <SID>detect_highlights()
augroup END

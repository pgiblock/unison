Basic Ideas
===========

These ideas are some of the original ideas I had for Unison.  Many of these
concepts are no longer valid after realizing some the design considerations
while implementing the curring Graph system, but for the most part this stuff is
right.



File System
-----------

Unison will use Qt for File I/O.  One feature provided by Qt is the ability to
"map" locations.  This can be used to allow the user to choose
locations for various resources.

### Styles
The data dir for the current style can be mounted to `style:`. This will allow
referencing images from CSS and widgets.

Other paths may come in handy.  But, this abstraction must not be used for
project resources such as presets, samples, and projects.  These are covered in
Resources.


Resources
---------

This needs more thought, but the general idea should be sound.

Unison will support an abstract resource framework inspired by LMMS.  Resources
can be identified by a unique code.  Unison will search for the resource in a
list of user specified providers.  The default list of providers will include:

* The current project
* The shared resource directory (`$PREFIX/share/unison/resources`)
* The user's resource directory (`$HOME/unison/resources`)
* The official Unison repository (`http://unison-studio.org/stuff`)

Of course, the default path names will be finalized later.

Although this is enough information to identify a file, it isn't enough to
describe the file to the user in the case that the file can not be resolved.
For example, the file may have originally existed on another user's computer.
Or the file may be online but the user doesn't have internet access.  In these
cases, some meta-data should be provided when a resource (such as a project or
preset) references another resource.  Therefore, Project files should contain
some sort of digest containing metadata about all distinct resources.  This
digest can be populated by querying the resource provider when saving a
project file.


Project Files
-------------

Project files will be a simple directory or ZIP based format.  Inside the
project will be at least the following:

* __A Descriptor file__- possible values are the project's creator, type,
  version, etc..  Perhaps merge this with the resource-digest (below).

* __A Resource digest__- enumerates all the distinct resources used in the 
  project.  This can be used to give the user hints when trying to resolve
  missing resources.  This will also contain the meta data for internal 
  resources, so that the editor only needs to read one file out of the ZIP in
  order to index the included resources.

* __One (or more) resources__- the directory/ZIP will contain at least one
  actual resource.  These resources are called internal resources.  Internal
  resources have the highest priority when resolving from resource-id.  (On
  second thought, maybe they should be one of the lowest priorities..)  The idea
  right now, is that one resource will be flagged as the "main resource" within
  the Descriptor file.  This is the resource opened automatically when Unison
  opens a project file.  All other resources in the project are treated equally
  by the resource-browser while indexing and searching.  See 'saving project
  files' below for details on how resources are packaged into a project
  @emph{file}.

* __Data files__- for Unison extensions such as "notepad" ?

When saving a file, there will probably be a few preset behaviors, but the
overall functionality will be this:

* __Save the project.__  This doesn't cause any external resources to be
  copied into the project (no external resources become internal resources).
  The project is a directory for speed-sake.  This allows live recording, etc..
  to be saved directly into the project.

* __Export the project.__  You can export as a directory, or archive file
  (ZIP?) The user is presented with a checkbox-list on export that lets them
  choose which resources to include in the project.  Any 'factory files' should
  be automatically unchecked.  Any resources already internal should be checked.
  This leaves non-internal, non-factory files to determine a default value for.

* __Exporting a sub-project.__ ...


### Thoughts
It may be that we want resources internal to the project
to actually have lower priority than other local-resource (resources on the hard
drive).

This all needs to be rethought to a degree because "resource-id" will change
through different versions of a file --- in some cases, this is what we want
in other cases it is not what we want. 

*The resource-id should actually be a more explicit identifier.
`<some_constant_id>-<version_id>`...*


Plugins
-------

The goal is to use LV2 plugins whenever appropriate.  We may need to make
something like gtkgui (QtGui?) to handle the GUI and maybe another optional
extension for Unison-specific features.

Instruments will still be represented as first-class objects in the GUI.
However, they will just be LV2 plugins.  The instrument is an insert in its
own FX line.  The FX line will probably contain a basic 3-pole EQ / amplifier,
the cutoff filter, and will have an FX chain. The FX line sends.  The Filters
could even be discrete plugins perhaps --- maybe even pluggable implementations.

The mixer is just another plugin.  The mixer and all send-groups are just
instances of a multi-in single-out plugin.  The plugins will be treated as a
directed graph.  But, the GUI treats them as first class objects and makes
them easily accessible.

The LV2 plugins can also be used for automation-controllers I think. Things like
the arpeggio will just be MIDI insert-filters that the user gets "for free".

The URI naming convention is quite handy for project files.  In fact, I am
considering using the same, or a simialar concept for Unison's resource
naming (as opposed to filehashes, which change uncontrollably with time).


Piano Roll
----------

The piano roll will be implemented, like most editors, as a QGraphicsScene. The
piano roll can be opened as its own window/pane (however we decide to display
editors) This will allow advanced control of the editor.  The piano roll can
also be accessed when a MIDI track is zoomed in deep enough.


Meter Changes
-------------

Changes in meter (time signature) will be obtained by maintaining a "MetricMap"
within the song/sequence.  The MetricMap holds a sorted list of MetricSections.
MeterSections and TempoSections represent changes in meter and tempo
respectively.  A MetricSection is positioned by both frame# and BBT time.  This
allows for more optimised recalculation when the metricMap is modified.  It also
allows for two different indexes when calculatingconversions between frames and
BBT time.

It would be nice to have something like time-warp in Cubase, I think our metric
map interface is capable of adding that complexity to down the road.


Messages
--------

Instead of printf and MessageBoxes, we should have a messages features.  In the
GUI, messages are delivered to a pane (or if the severity is high enough, then
perhaps to a messagebox).  In the CLI, messages can be delivered to stdout and
stderr.  This centralizes the information we deliver to the user, eliminates the
flood of MessageBoxes when loading a project with many missing resources. Also,
this can help with logging, all messages can be logged to disk, and we could
allow additional data to be logged for debugging purposes.


Key
---

It would be nice is there can be a single track/timeline for storing the key of
the song.  This can be used to grey-out notes that aren't in key while working
in the piano-roll.


Dependency Graph
----------------

Dependencies between plugins (I guess, more specifically, their ports) will be
managed as a directed graph.  This will allow for traversal of dependencies and
dependants.  We will generate a traversal of plugins to be rendered, based on
the graph.  This list can then be atomically swapped into the processing thread.
Perhaps, down the road, we can generate multiple lists and assign entire lists
to each worker thread.

Like most things, the dependency graph is a low-level abstraction. Unison will
represent different components of the graph differently. Also, there may be
cases where a single GUI element actually represents a group of plugins.
Example: A non-wirable synth built on oscillator, filter, and envelope plugins.


Tracks
------

Unison uses the common "track" concept to model the composition. A single song
can contain many tracks, each one is used to control different things with
respect to time.  A track contains multiple segments.  The type of segment
varies dending on the track type.  Here are a few tracks that will be included
with Unison:

* __Instrument Track__- Contains Patterns. Each pattern contains a sequence
  of MIDI events.  These are represented to the user as notes in a piano roll,
  and as notes in the track-segment itself.  The track has a single instrument
  associated with it, the track is the sole MIDI insert for that instrument.
  Perhaps this one-to-one restriction can be broadened as we get more
  comfortable with the architecture of Unison.

* __Automation Track__- Contains automation data for a control. Automation
  data can be recorded live and also be created or modified using the automation
  editor and inline editor in the song editor.  When recording automation,
  Unison can track changes in the GUI, changes to associated MIDI controls
  (means of connecting controllers is not determined yet), or by recording the
  generated values of an attached internal controller (such as LFO).  The user
  will need to "arm" the track for recording prior to it writing new events.
  Upon storing events, the system will try to simply the automation into vector
  parts according to the user's current "simplify automation recording" level
  setting.  The recording will be more "true" with a smaller simplification
  level.

* __Audio Track__- Contains clips of audio.  The audio can reference
  external files, or the audio can be recorded straight within Unison (from
  audio inserts).  If the user has "audio takes" set, then looping over a
  section of audio while recording will cause a stack of multiple "takes" to be
  recorded the user can then choose or crossfade between the different takes as
  desired.

* __BB Track or Folder Track__- I am unsure if I really like the BB Editor
  from LMMS or FL Studio.  This, however, could be the way we decide to group
  similar parts.  This allows the song editor to be more organized, and allows
  for looping.  Another possiblity is to allow for Folder-tracks.  A folder
  track can contain multiple tracks. The folder can be collapsed or expanded.
  Certain actions performed on the group will be propagated to the children
  tracks (such as cutting, muting, and repositioning of segments).

* __Layer Track__- This is a popular feature from FL Studio, and is
  frequently requested by LMMS users.  This allows multiple instruments to be
  contained in a group.  All instruments in the layer play the same MIDI events.
  Not only does this allow for more lush sounds, but it also allows saving the
  layer as a preset to easily use in new projects.  Perhaps Layer Tracks can be
  realized as a specific kind of "Folder Track", except, only Instrument tracks
  are allowed as children, and the children cannot have their own patterns.


Mixer
-----

This is a very grey area right now, at best.  There are lots of mixer models out
on the market right now --- all with varying degrees of flexibility and
complexity.  In my opinion, the Mixer in FL Studio 7 was very limiting.  4 Send
channels was too much of a restriction.  the FLS mixer did support "inserts"
to a degree, in the sense that multiple instruments could choose the same "FX
channel".  This model works well, but it is odd having a difference between
inserts from instruments and inserts from other FX lines (the sends).

Real hardware mixers have all sorts of features that are overkill for a studio
that tries to be accessible.  Pre-fader inserts and sends, post-fader AUX send
and returns...  Things start getting complicated.

An important design consideration is that we do not limit the ability to do
things like side chaining and sending.  Pre-fader sends could be nice for things
like vocoders --- but maybe could be implemented differently?  Another nice
feature to reserve is the ability to monitor (non-master output).  This is
another area where pre-fader sends are handy (listen to the singers, or an
instrument before the FX and fading is applied).

As stated before, I do not like "FX channel" selector on instruments.  It is
inconsistent with FX sends, and it limits the instrument to sending to a single
FX channel.  I'd like instruments to be able to dynamically add and remove
outputs.  For example, a "Slicer" instrument that cuts a wave into multiple
parts could allow the user to expose a seperate output for each slice instead of
a single output for the whole instrument.  Therefore, I vote for each output on
each instrument or audio track to be represented as a dedicated FX line in the
mixer.  This takes the place (or duplicates) the functionality provided by the
Instrument Window's VOL and PAN knobs as well as the FX chain.

There may be a lot of FX lines in this case, so means of grouping the FX lines
for organizational purposes should be included.

Comments regarding the mixer are greatly appreciated!


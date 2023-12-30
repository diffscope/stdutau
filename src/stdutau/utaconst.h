#ifndef UTACONST_H
#define UTACONST_H

namespace Utau {

    // Sections
    constexpr const char SECTION_BEGIN_MARK[] = "[#";
    constexpr const char SECTION_END_MARK[] = "]";
    constexpr const char SECTION_NAME_DELETE[] = "DELETE";
    constexpr const char SECTION_NAME_INSERT[] = "INSERT";
    constexpr const char SECTION_NAME_NEXT[] = "NEXT";
    constexpr const char SECTION_NAME_PREV[] = "PREV";
    constexpr const char SECTION_NAME_SETTING[] = "SETTING";
    constexpr const char SECTION_NAME_VERSION[] = "VERSION";
    constexpr const char SECTION_NAME_TRACKEND[] = "TRACKEND";

    // Version
    constexpr const char UST_VERSION_1_2[] = "1.2";
    constexpr const char UST_VERSION_PREFIX[] = "UST Version ";

    // Settings
    constexpr const char KEY_NAME_PROJECT_NAME[] = "ProjectName";
    constexpr const char KEY_NAME_TRACKS[] = "Tracks";
    constexpr const char KEY_NAME_OUTPUT_FILE[] = "OutFile";
    constexpr const char KEY_NAME_MODE2[] = "Mode2";
    constexpr const char KEY_NAME_TOOL1[] = "Tool1";
    constexpr const char KEY_NAME_TOOL2[] = "Tool2";

    // Note
    constexpr const char KEY_NAME_CHARSET[] = "Charset";
    constexpr const char KEY_NAME_PROJECT[] = "Project";
    constexpr const char KEY_NAME_CACHE_DIR[] = "CacheDir";
    constexpr const char KEY_NAME_DIRECT[] = "$direct";
    constexpr const char KEY_NAME_ENVELOPE[] = "Envelope";
    constexpr const char KEY_NAME_FLAGS[] = "Flags";
    constexpr const char KEY_NAME_INTENSITY[] = "Intensity";
    constexpr const char KEY_NAME_LENGTH[] = "Length";
    constexpr const char KEY_NAME_LYRIC[] = "Lyric";
    constexpr const char KEY_NAME_MODULATION[] = "Modulation";
    constexpr const char KEY_NAME_MODURATION[] = "Moduration";
    constexpr const char KEY_NAME_NOTE_NUM[] = "NoteNum";
    constexpr const char KEY_NAME_PBM[] = "PBM";
    constexpr const char KEY_NAME_PBS[] = "PBS";
    constexpr const char KEY_NAME_PB_TYPE[] = "PBType";
    constexpr const char KEY_NAME_PB_START[] = "PBStart";
    constexpr const char KEY_NAME_PBW[] = "PBW";
    constexpr const char KEY_NAME_PBY[] = "PBY";
    constexpr const char KEY_NAME_PICHES[] = "Piches";
    constexpr const char KEY_NAME_PITCHES[] = "Pitches";
    constexpr const char KEY_NAME_PITCH_BEND[] = "PitchBend";
    constexpr const char KEY_NAME_PRE_UTTERANCE[] = "PreUtterance";
    constexpr const char KEY_NAME_START_POINT[] = "StartPoint";
    constexpr const char KEY_NAME_TEMPO[] = "Tempo";
    constexpr const char KEY_NAME_VBC[] = "VBC";
    constexpr const char KEY_NAME_VBR[] = "VBR";
    constexpr const char KEY_NAME_VELOCITY[] = "Velocity";
    constexpr const char KEY_NAME_VOICE_DIR[] = "VoiceDir";
    constexpr const char KEY_NAME_VOICE_OVERLAP[] = "VoiceOverlap";
    constexpr const char KEY_NAME_LABEL[] = "Label";
    constexpr const char KEY_NAME_PATCH[] = "$patch";
    constexpr const char KEY_NAME_REGION_START[] = "$region";
    constexpr const char KEY_NAME_REGION_END[] = "$region_end";
    constexpr const char KEY_NAME_PRE_UTTERANCE_READONLY[] = "@preuttr"; // Readonly
    constexpr const char KEY_NAME_VOICE_OVERLAP_READONLY[] = "@overlap";
    constexpr const char KEY_NAME_START_POINT_READONLY[] = "@stpoint";
    constexpr const char KEY_NAME_FILENAME_READONLY[] = "@filename";
    constexpr const char KEY_NAME_ALIAS_READONLY[] = "@alias";
    constexpr const char KEY_NAME_CACHE_READONLY[] = "@cache";

    // Values
    constexpr const double DEFAULT_VALUE_INTENSITY = 100.0;
    constexpr const double DEFAULT_VALUE_MODULATION = 100.0;
    constexpr const double DEFAULT_VALUE_PRE_UTTERANCE = 0.0;
    constexpr const double DEFAULT_VALUE_TEMPO = 120.0;
    constexpr const double DEFAULT_VALUE_VOICE_OVERLAP = 0.0;

    constexpr const double DEFAULT_VALUE_VELOCITY = 100.0;
    constexpr const double DEFAULT_VALUE_START_POINT = 0.0;
    constexpr const char DEFAULT_VALUE_FLAGS[] = "";

    constexpr const char VALUE_MODE2_ON[] = "True";
    constexpr const char VALUE_TRACKS_SINGLE[] = "1";
    constexpr const char VALUE_PITCH_TYPE[] = "5";

    // Utils
    constexpr const char TONE_NAMES[] = "CCDDEFFGGAAB";
    constexpr const char TONE_NAME_SHARP = '#';

    constexpr const int TIME_BASE = 480;
    constexpr const int TONE_NUMBER_BASE = 24;
    constexpr const int TONE_OCTAVE_MAX = 7;
    constexpr const int TONE_OCTAVE_MIN = 1;
    constexpr const int TONE_OCTAVE_STEPS = 12;

}

#endif // UTACONST_H

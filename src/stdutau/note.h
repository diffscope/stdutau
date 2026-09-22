#ifndef NOTE_H
#define NOTE_H

#include <array>
#include <map>
#include <string>
#include <vector>
#include <optional>

#include <stdutau/utaglobal.h>
#include <stdutau/utaconst.h>

namespace utau {

    /// One anchor of a pitch curve or an envelope, with the shape of the curve leading to it.
    class STDUTAU_EXPORT Point {
    public:
        /// The curve shape from the previous anchor to this one. The names follow UTAU.
        ///
        /// \warning The letter in \c PBM is not the initial of the name. \c SJoin is written as
        ///          an empty string, and \c s denotes \c LinearJoin .
        enum Type {
            SJoin,      ///< an S curve, written as an empty string
            LinearJoin, ///< a straight line, written as \c s
            RJoin,      ///< steep at the start, written as \c r
            JJoin,      ///< steep at the end, written as \c j
        };

        inline constexpr Point();
        inline constexpr Point(double x, double y);
        inline constexpr Point(double x, double y, Type p);

        inline constexpr bool operator==(const Point &other) const;
        inline constexpr bool operator!=(const Point &other) const;

        /// Orders by \a x only, as required for sorting a curve into time order.
        inline constexpr bool operator<(const Point &other) const;

        /// Any value other than the three letters is read as \c SJoin .
        static Type stringToType(const std::string_view &s);

        /// Returns an empty string for \c SJoin , which is how UTAU writes it.
        static std::string typeToString(Type type);

    public:
        /// Milliseconds from the start of the note. Negative if the curve extends into the
        /// preceding note.
        double x;

        /// Height, in tenths of a semitone for a pitch curve and in percent for an envelope.
        double y;

        /// Unused in an envelope, whose curve shape is fixed.
        Type type;
    };

    inline constexpr Point::Point() : Point(0.0, 0.0, SJoin) {
    }

    inline constexpr Point::Point(double x, double y) : Point(x, y, SJoin) {
    }

    inline constexpr Point::Point(double x, double y, Type t) : x(x), y(y), type(t) {
    }

    inline constexpr bool Point::operator==(const Point &other) const {
        return (other.x == x && other.y == y && other.type == type);
    }

    inline constexpr bool Point::operator!=(const Point &other) const {
        return !((*this) == other);
    }

    inline constexpr bool Point::operator<(const Point &other) const {
        return x < other.x;
    }

    /// The vibrato of a note, stored in the \c VBR entry.
    class STDUTAU_EXPORT Vibrato {
    public:
        inline constexpr Vibrato();

        /// The eight comma-separated values of \c VBR .
        std::string toString() const;

        /// Returns a default vibrato if \a s has fewer than seven values. The eighth value is
        /// optional and is read as zero if absent.
        static Vibrato fromString(const std::string_view &s);

    public:
        /// The vibrato length, in percent of the note, measured backward from its end.
        double length;

        /// The period in milliseconds.
        double period;

        /// The depth in cents.
        double amplitude;

        /// Fade-in and fade-out, each in percent of \a length.
        double attack;
        double release;

        /// The starting phase within the cycle, in percent.
        double phase;

        /// The offset of the vibrato from the note pitch, in percent of \a amplitude.
        double offset;

        /// Read and written so that a round trip preserves it, but otherwise unused.
        double intensity;
    };

    inline constexpr Vibrato::Vibrato()
        : length(65), period(180), amplitude(35), attack(20), release(20), phase(0), offset(0),
          intensity(0) {
    }

    /// The volume envelope of a note, stored in the \c Envelope entry.
    class STDUTAU_EXPORT Envelope {
    public:
        inline constexpr Envelope();

        /// Four anchors, or five if the optional middle anchor is used.
        inline constexpr int count() const;

        /// The anchors in the order of the \c Envelope entry, which differs from the order in
        /// this structure, with the \c % separator if a fifth anchor requires it.
        std::string toString() const;

        /// Returns a default envelope if \a s has fewer than seven values.
        static Envelope fromString(const std::string_view &s);

    public:
        /// In time order, filled from index zero, so a note with four anchors leaves index four
        /// at its default of (-1, -1), which count() evaluates. The fifth anchor, if present, is
        /// the additional middle anchor and is stored at index two.
        ///
        /// Point::type is unused here.
        std::array<Point, 5> anchors;
    };

    inline constexpr Envelope::Envelope()
        : anchors({Point(0, 0), Point(5, 100), Point(35, 100), Point(0, 0), Point(-1, -1)}) {
    }

    inline constexpr int Envelope::count() const {
        return anchors[4].y >= 0 ? 5 : 4;
    }

    /// One note of a track, corresponding to one numbered section of a UST.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \note The official UTAU site publishes no format specification. The page below defines
    ///       the entries of the plugin temporary file, which are the entries of a UST.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様
    class STDUTAU_EXPORT Note {
    public:
        inline Note();

        /// Constructs a note as an editor does, so that the values an editor sets explicitly are
        /// present rather than absent.
        Note(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);

        virtual ~Note() = default;

        /// The value used for rendering, which is the default if the note specifies none.
        inline constexpr double realIntensity() const;
        inline constexpr double realModulation() const;
        inline constexpr double realVelocity() const;
        inline constexpr double realStartPoint() const;

        /// The duration in milliseconds of \a length ticks at \a tempo.
        static inline constexpr double duration(int length, double tempo);

    public:
        /// \c R and the empty string denote rests, as determined by isRestLyric().
        std::string lyric;

        /// Appended to the project flags rather than replacing them.
        std::string flags;

        /// The key, where 24 is C1.
        int noteNum;

        /// The length in ticks, where 480 is a quarter note.
        int length;

        /// Absent if the file omits the entry, which in UTAU means that the value of the project
        /// or the voice bank applies.
        std::optional<double> intensity, modulation, velocity;
        std::optional<double> preUttr, overlap, stp;
        std::optional<double> tempo;

        std::optional<Envelope> envelope;

        /// The mode 2 pitch, a curve of anchors. Empty if the note has none.
        std::vector<Point> portamento;
        std::optional<Vibrato> vibrato;

        /// The mode 1 pitch, one value every five ticks, starting at \a pbstart . A project uses
        /// exactly one mode, so at most one of this and \a portamento is filled.
        std::optional<double> pbstart;
        std::vector<double> pitches;
        std::string pbtype;

        /// User text attached to the note, displayed by UTAU in the piano roll.
        std::string label;

        /// Whether the sample is rendered unmodified, without the resampler.
        std::string direct;

        /// A sample used in place of the sample resolved from the lyric.
        ///
        /// \warning A path from the project file, and therefore untrusted. Passing it to a process
        ///          unchecked reproduces CVE-2024-28886.
        std::string patch;

        /// The name of the region this note opens or closes, displayed by UTAU as a labeled
        /// range of the piano roll.
        std::string region, regionEnd;

        /// Entries of the note without a dedicated member. They are preserved as read and written
        /// back at the end of the section, so that a host can store its own data on a note.
        ///
        /// The map is keyed by entry name, so a repeated name resolves to the last value read.
        /// UTAU resolves it the same way.
        ///
        /// \note UTAU preserves an unrecognized entry only if its name begins with \c $ , and only
        ///       in a note section. Any other entry stored here survives a round trip through this
        ///       library but is lost once the file is saved by UTAU. UTAU also restricts the value:
        ///       \c = and a tab truncate it, a space is replaced with a comma, and an empty value
        ///       removes the entry.
        std::map<std::string, std::string> userData;
    };

    inline Note::Note() : Note(60, 480) {
    }

    inline constexpr double Note::realIntensity() const {
        return intensity.value_or(DEFAULT_VALUE_INTENSITY);
    }

    inline constexpr double Note::realModulation() const {
        return modulation.value_or(DEFAULT_VALUE_MODULATION);
    }

    inline constexpr double Note::realVelocity() const {
        return velocity.value_or(DEFAULT_VALUE_VELOCITY);
    }

    inline constexpr double Note::realStartPoint() const {
        return stp.value_or(DEFAULT_VALUE_START_POINT);
    }

    inline constexpr double Note::duration(int length, double tempo) {
        return length * 125.0 / tempo;
    }

    /// A note as stored in a plugin temporary file: a note plus the values UTAU computed for it.
    ///
    /// The additional members are read-only. Modifying them in a plugin has no effect, because
    /// UTAU recomputes them from the note and the voice bank.
    class NoteExt : public Note {
    public:
        inline NoteExt();
        inline NoteExt(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);

    public:
        /// The values UTAU computed for the note, absent if the file does not specify them.
        std::optional<double> preUttrRO;
        std::optional<double> overlapRO;
        std::optional<double> stpRO;

        /// Empty if the file does not specify it. A sample without an alias has an empty alias,
        /// so an optional would add no distinction.
        std::string filenameRO;
        std::string aliasRO;
        std::string cacheRO;
    };

    inline NoteExt::NoteExt() {
    }

    inline NoteExt::NoteExt(int noteNum, int length, const std::string &lyric)
        : Note(noteNum, length, lyric) {
    }

    /// The four entries that together store a mode 2 pitch curve, as they appear in the file.
    ///
    /// Note::portamento holds the same curve as points and is the form intended for use. This
    /// structure is the serialized form used when reading and writing.
    struct STDUTAU_EXPORT PBStrings {
        /// The starting point of the curve, as \c x;y .
        std::string PBS;

        /// The distance from each anchor to its predecessor, comma-separated.
        std::string PBW;

        /// The height of each anchor after the first, comma-separated. An empty field is zero.
        std::string PBY;

        /// The Point::Type of each anchor after the first, comma-separated.
        std::string PBM;

        /// Returns the curve as points with absolute positions, or an empty list if \a PBS or
        /// \a PBW is empty. An anchor positioned before its predecessor is moved to the position of
        /// its predecessor.
        std::vector<Point> toPoints() const;

        /// Returns the four entries for \a points , or empty strings if \a points is empty.
        static PBStrings fromPoints(const std::vector<Point> &points);
    };

}

#endif // NOTE_H

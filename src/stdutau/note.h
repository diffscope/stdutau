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

    /// One anchor of a pitch curve or an envelope, with the shape of the curve reaching it.
    class STDUTAU_EXPORT Point {
    public:
        /// How the curve runs from the previous anchor to this one. The names are UTAU's.
        ///
        /// \warning The letter \c PBM writes is not the initial of the name. \c SJoin is the one
        ///          written as nothing at all, and \c s stands for \c LinearJoin .
        enum Type {
            SJoin,      ///< an S curve, written as nothing at all
            LinearJoin, ///< a straight line, written as \c s
            RJoin,      ///< steep at the start, written as \c r
            JJoin,      ///< steep at the end, written as \c j
        };

        inline constexpr Point();
        inline constexpr Point(double x, double y);
        inline constexpr Point(double x, double y, Type p);

        inline constexpr bool operator==(const Point &other) const;
        inline constexpr bool operator!=(const Point &other) const;

        /// Orders by \a x alone, which is what sorting a curve into time order wants.
        inline constexpr bool operator<(const Point &other) const;

        /// Anything that is not one of the three letters reads as \c SJoin .
        static Type stringToType(const std::string_view &s);

        /// Returns an empty string for \c SJoin , which is how UTAU writes it.
        static std::string typeToString(Type type);

    public:
        /// Milliseconds from the start of the note, which may be negative where the curve reaches
        /// back into the note before.
        double x;

        /// Height. A pitch curve reads it in tenths of a semitone, an envelope as a percentage.
        double y;

        /// Unused in an envelope, whose shape is fixed.
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

    /// A note's vibrato, which is the \c VBR entry.
    class STDUTAU_EXPORT Vibrato {
    public:
        inline constexpr Vibrato();

        /// The eight values comma separated, as \c VBR holds them.
        std::string toString() const;

        /// Returns a default vibrato when \a s has fewer than seven values. The eighth is
        /// optional and reads as zero when missing.
        static Vibrato fromString(const std::string_view &s);

    public:
        /// How much of the note vibrates, as a percentage counted back from its end.
        double length;

        /// One cycle in milliseconds.
        double period;

        /// Depth in cents.
        double amplitude;

        /// Fade in and fade out, each a percentage of \a length.
        double attack;
        double release;

        /// Where in the cycle the vibrato starts, as a percentage.
        double phase;

        /// Shifts the whole vibrato off the note's pitch, as a percentage of \a amplitude.
        double offset;

        /// Read and written so that a round trip keeps it, but nothing acts on it.
        double intensity;
    };

    inline constexpr Vibrato::Vibrato()
        : length(65), period(180), amplitude(35), attack(20), release(20), phase(0), offset(0),
          intensity(0) {
    }

    /// A note's volume envelope, which is the \c Envelope entry.
    class STDUTAU_EXPORT Envelope {
    public:
        inline constexpr Envelope();

        /// Four anchors, or five where the optional middle one is in use.
        inline constexpr int count() const;

        /// The anchors in the order \c Envelope holds them, which is not the order they sit in
        /// here, with the \c % separator where a fifth anchor calls for one.
        std::string toString() const;

        /// Returns a default envelope when \a s has fewer than seven values.
        static Envelope fromString(const std::string_view &s);

    public:
        /// In time order, filled from index zero, so a note using four of them leaves index four
        /// at its default of (-1, -1). That is what count() reads. The fifth anchor, where a note
        /// has one, is the extra middle one and lands at index two.
        ///
        /// Point::type goes unused here.
        std::array<Point, 5> anchors;
    };

    inline constexpr Envelope::Envelope()
        : anchors({Point(0, 0), Point(5, 100), Point(35, 100), Point(0, 0), Point(-1, -1)}) {
    }

    inline constexpr int Envelope::count() const {
        return anchors[4].y >= 0 ? 5 : 4;
    }

    /// One note of a track, which is one numbered section of a UST.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    ///
    /// \note UTAU's own site publishes no format document. The page below defines the entries
    ///       of the temporary file a plugin is handed, which are the entries of a UST.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様
    class STDUTAU_EXPORT Note {
    public:
        inline Note();

        /// Builds a note as an editor would, so the values an editor sets explicitly are present
        /// rather than absent.
        Note(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);

        virtual ~Note() = default;

        /// The value to render with, which is the default where the note carries none.
        inline constexpr double realIntensity() const;
        inline constexpr double realModulation() const;
        inline constexpr double realVelocity() const;
        inline constexpr double realStartPoint() const;

        /// Milliseconds a note of \a length ticks lasts at \a tempo.
        static inline constexpr double duration(int length, double tempo);

    public:
        /// \c R and the empty string are rests, which isRestLyric() settles.
        std::string lyric;

        /// Appended to the project flags rather than replacing them.
        std::string flags;

        /// Key, where 24 is C1.
        int noteNum;

        /// Ticks, where 480 is a quarter note.
        int length;

        /// Absent where the file leaves the entry out, which is how UTAU says to use the value
        /// from the project or the voice bank instead.
        std::optional<double> intensity, modulation, velocity;
        std::optional<double> preUttr, overlap, stp;
        std::optional<double> tempo;

        std::optional<Envelope> envelope;

        /// Pitch as mode 2 keeps it, a curve of anchors. Empty where the note has none.
        std::vector<Point> portamento;
        std::optional<Vibrato> vibrato;

        /// Pitch as mode 1 keeps it, one reading every five ticks, beginning where \a pbstart
        /// says. A project is in one mode or the other, so either these or \a portamento is
        /// filled, not both.
        std::optional<double> pbstart;
        std::vector<double> pitches;
        std::string pbtype;

        /// Text a user attached to the note, which UTAU shows in the piano roll.
        std::string label;

        /// Render the sample as it stands, without the resampler.
        std::string direct;

        /// A sample to use in place of the one the lyric resolves to.
        ///
        /// \warning This is a path out of the project file, so treat it as untrusted. Handing it
        ///          to a process as it stands is CVE-2024-28886.
        std::string patch;

        /// Names the region this note opens or closes, which UTAU shows as a labelled stretch of
        /// the piano roll.
        std::string region, regionEnd;

        /// Entries found on the note that this class does not otherwise represent. They are kept
        /// as they were read and written back at the end of the section, so a host can store its
        /// own data on a note and find it again.
        ///
        /// The map is keyed by the entry name, which makes a repeated name resolve to the last
        /// one read. UTAU resolves it the same way.
        ///
        /// \note UTAU keeps an entry it does not know only when the name begins with \c $ , and
        ///       only on a note section. Anything else stored here survives a round trip through
        ///       this library and is gone the moment the file passes through UTAU. The value is
        ///       subject to further limits there: \c = and a tab truncate it, a space becomes a
        ///       comma, and an empty value drops the entry.
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

    /// A note as a plugin temporary file carries it, which is a note plus what UTAU worked out
    /// about it.
    ///
    /// The extra members are read only. A plugin changing one of them changes nothing, since UTAU
    /// works them out again from the note and the voice bank.
    class NoteExt : public Note {
    public:
        inline NoteExt();
        inline NoteExt(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);

    public:
        /// What UTAU worked out for the note, absent where the file did not say.
        std::optional<double> preUttrRO;
        std::optional<double> overlapRO;
        std::optional<double> stpRO;

        /// Empty where the file did not say. An empty alias is what a sample without one has, so
        /// there is nothing here for an optional to tell apart.
        std::string filenameRO;
        std::string aliasRO;
        std::string cacheRO;
    };

    inline NoteExt::NoteExt() {
    }

    inline NoteExt::NoteExt(int noteNum, int length, const std::string &lyric)
        : Note(noteNum, length, lyric) {
    }

    /// The four entries a mode 2 pitch curve is spread across, as they appear in the file.
    ///
    /// Note::portamento is the same curve as points, which is what to work with. This is the
    /// shape it takes on the way in and out.
    struct STDUTAU_EXPORT PBStrings {
        /// Where the curve starts, as \c x;y .
        std::string PBS;

        /// The gap from each anchor to the one before it, comma separated.
        std::string PBW;

        /// The height of each anchor after the first, comma separated. An empty field is zero.
        std::string PBY;

        /// The Point::Type of each anchor after the first, comma separated.
        std::string PBM;

        /// Returns the curve as points with absolute positions, empty when \a PBS or \a PBW is.
        /// An anchor landing before the one ahead of it is pulled forward to meet it.
        std::vector<Point> toPoints() const;

        /// Returns the four entries for \a points, empty strings when \a points is empty.
        static PBStrings fromPoints(const std::vector<Point> &points);
    };

}

#endif // NOTE_H

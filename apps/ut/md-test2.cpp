/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include "metadata/traits.tcc"
# include "analysis/evSource_sectional.tcc"

namespace sV {
namespace test {

// (Einsturzende Neubauten, "Nagorny Karabach", 2007)
const char _static_srcPoe[][512] = {
R"(Die Stadt liegt unter Nebel
Ich bin auf meinem Berg
In meinem schwarzen Garten
zwischen Himmeln eingeklemmt
in der Enklave meiner Wahl
in der ich mich versteck
in Nagorny Karabach

Vormals tiefe Wälder
Bergketten, vielleicht Eis
eine messinggelbe Sonne
verbricht ein Paradies
meine Sys- und Diastole
dazwischen der Moment
getragen von den Vögeln
die hier zugange sind
in der Enklave meines Herzens
in der ich mich verlier
in Nagorny Karabach)",

R"(Ich steig den Berg herunter
geh ins eine oder andere Tal
es ist geflaggt in allen Farben
in Bergisch-Karabach

Zwei grosse schwarze Raben
fressen die Pflaumen aus dem Baum
Ob die andre Stadt mich lieb hat...?)",

R"(In der Enklave meiner Wahl
in der ich mich verberg'
in Nagorny Karabach

Komm mich mal besuchen
ich hab' unendlich Zeit
und der Blick der ist vom Feinsten
über Wolken und die Stadt
in Nagorny Karabach
Nagorny Karabach
Nagorny Karabach)"
};

// A second "EventID" is somewhat more complicated --- it is location of the
// word identifying fragment, number of stanza inside fragment, line number
// inside stanza, and the word number inside it.
struct EventID {
    unsigned char fragmentNo;
    unsigned char stanzaNo, lineNo;
    size_t wordNo;
};
// Corresponding metadata type has to denote the fragment number and word
// offset in it:
struct Metadata {
    const char * fragmentPtr;
    size_t offset;
};
// Considering source identifier as a just pointer to fragment, let us define
// the trats type:
typedef sV::MetadataTypeTraits<EventID, Metadata, const char *> MetadataTraits;

}  // namespace test
}  // namespace sV


# T.E.A.L.-TK---1176-Compressor-
T.E.A.L.  TK - 1176 Compressor  

---

## 🎛️ The plugin: TEAL 1176

A working **VST3 / Standalone** FET feedback compressor–limiter that implements the
model researched in this document. Built with JUCE + CMake.

- **Build it:** see **[BUILDING.md](BUILDING.md)** — `cmake -B build && cmake --build build`
- **DSP core:** [`source/dsp/`](source/dsp) — pure C++ (no JUCE), unit-testable offline
- **Plugin:** [`source/PluginProcessor.cpp`](source/PluginProcessor.cpp), [`source/PluginEditor.cpp`](source/PluginEditor.cpp)

Controls: **Input** (drive), **Output** (make-up), **Attack** (20–800 µs),
**Release** (50 ms–1.1 s), **Ratio** (4 / 8 / 12 / 20 / All-buttons), **Mix**,
**Oversampling** (1×–8×), Bypass — with a VU-style gain-reduction meter.

What it captures from the hardware: feedback detection (ratio `R = 1+k`, soft knee,
program-dependent ratio creep), the even-harmonic FET/transformer colour, the
all-buttons "British mode" grind, retained rectifier ripple for low-frequency grit,
ADAA + oversampling antialiasing, and zero added latency (feedback ⇒ no lookahead).
The rest of this README is the research and DSP derivations that back it.

---

Deep Research on the 1176 Compressor
The Universal Audio 1176 Limiting Amplifier is one of the most influential compressors in music production history, known for its fast attack time (20 µs), FET-based circuitry, and distinctive sonic character blog.insideblackbird.com+1.

Origins and Design
Introduced in 1967 by Bill Putnam Sr. of Universal Audio, the 1176 was a direct evolution of the tube-based 175 and 176 variable‑mu compressors Wikipedia. It marked a shift from vacuum tubes to solid‑state Field‑Effect Transistor (FET) technology, offering speed, consistency, and a unique tonal coloration blog.insideblackbird.com+1.
The compressor is a feedback‑type design: the FET acts as a variable resistor, shunting more signal to ground as gain reduction increases blog.insideblackbird.com. It uses soft knee compression with a fixed threshold (set via input gain) and four selectable ratios (4:1, 8:1, 12:1, 20:1) Wikipedia+1. The “All Buttons In” mode adds a distinctive distortion by engaging all ratio settings simultaneously soundcy.com.

Key Technical Features
Attack/Release: Continuously variable, with attack as fast as 20 µs — revolutionary for its time blog.insideblackbird.com+1.

Output: Class‑A amplifier with a custom output transformer (recreated by Universal Audio) blog.insideblackbird.com.

Controls: Two large Input/Output knobs, two smaller Attack/Release knobs, and four push‑button ratio switches blog.insideblackbird.com.

Release: Program‑dependent — faster after transients, slower after sustained compression to reduce pumping Wikipedia.

Revisions and Variations
The 1176 went through multiple revisions, each with subtle but notable changes VOX Recording Studios+1:

Rev A (1176LN “Blue Stripe”): Early model with FET + Class‑A amp + custom transformer; prized for warmth, punch, and coloration VOX Recording Studios.

Rev B/C (“Silver Face”): Improved reliability, slightly more transparent, less coloration than Blue Stripe VOX Recording Studios.

Rev D/E/F (“Classic Black Face”): Mid‑1970s; refined performance and headroom VOX Recording Studios.

Rev H: Final original UREI version; later reissued by Universal Audio after 1999 Wikipedia.

Sonic Character
The 1176’s sound is often described as musical, punchy, and warm, with a rich harmonic distortion that adds “character” to vocals, guitars, bass, and mixes soundcy.com. Early models (Blue Stripe) are sought after for their vintage warmth, while later models (Silver Face) offer cleaner, more transparent compression VOX Recording Studios.

Legacy
Used on more records than any other compressor, the 1176 has been a staple in professional studios worldwide blog.insideblackbird.com. Its combination of speed, versatility, and tonal personality has made it a “desert island” compressor — essential for engineers who value both control and musicality blog.insideblackbird.com. Even in the digital age, hardware and plugin versions remain popular for achieving its signature sound insidetherecordingstudio.com.

In summary: The 1176 is a landmark in audio engineering — a fast, versatile, and character‑rich compressor whose design innovations and enduring sound have shaped generations of recordings.

1176 Peak Limiter

Article
Talk
Read
Edit
View history

Tools
Appearance hide
Text

Small

Standard

Large
Width

Standard

Wide
Color

Automatic

Light

Dark
From Wikipedia, the free encyclopedia

This article provides insufficient context for those unfamiliar with the subject. Please help improve the article by providing more context for the reader. (November 2025) (Learn how and when to remove this message)
1176 Peak Limiter
1176 Limiting Amplifier

UA 1176 Revision A
Manufacturer	Universal Audio
Dates	1967–present
Technical specifications
Effects type	Compressor/Limiter
Hardware	Analog
Controls
Input/output
Inputs	1
Outputs	1
The 1176 Peak Limiter is a dynamic range compressor introduced by UREI in 1967. Derived from the 175 and 176 tube compressors, it marked the transition from vacuum tubes to solid-state technology.[1]

The limiter featured Class A amplifiers, input and output transformers, fast attack and release times, and different compression ratios and modes. The 2019 book Innovation in Music states that the 1176 was immediately appreciated by engineers and producers and established as a studio standard through the years.[2] At the time of its introduction, it was the first true peak limiter with all solid-state circuitry.[citation needed]

The 1176LN was inducted into the TECnology Hall of Fame in 2008.[3]

History

One of 1176 predecessors, UA 175 Limiting Amplifier
In 1966, the engineer Bill Putnam, founder of Universal Audio, began to employ the recently invented field-effect transistors (FET), replacing vacuum tubes in his equipment designs. After successfully adapting the 108 tube microphone preamplifier into the new FET-based 1108, he redesigned the 175 and 176 variable-mu tube compressors into the new 1176 compressor.

The initial units (A and AB revisions) were available in 1967 and were informally referred as "blue stripe" for their blue-colored meter section. Revision C, designed in 1970, saw one of the major design evolutions, with less noise and harmonic distortion. It was renamed to 1176LN and the face color changed to the now familiar solid black.[1]

Bill Putnam sold UREI in 1985 and Revision H was the last series produced by the original company. However, the company was re-established as Universal Audio in 1999 by the sons Bill Putnam, Jr. and Jim Putnam, and the company re-issued the 1176LN as its first product. The original design was reproduced and revised thanks to the extensive design notes left by Bill Putnam.[1]

Design

Six UREI 1176LN (revision H) compressors stacked in individual flight cases
The 1176 uses a field-effect transistor (FET) to obtain gain reduction arranged in a feedback configuration. As its predecessor, the 1176 utilizes soft knee compression[4] and fixed threshold: compression amount is controlled through the input control. The compression character is handled by attack and release times and four selectable compression ratios. The release time is program-dependent: it is quicker after transients to obtain a more consistent level, but it slows down after sustained and heavy compression to reduce pumping effects. The threshold is set higher on higher ratios.[1]

Four different compression ratios are available: 4:1, 8:1, 12:1, and 20:1
Attack time is adjustable from 20 μs to 800 μs (0.00002–0.0008 seconds)
Release times are adjustable from 50 ms to 1100 ms (0.05–1.1 seconds)
Two units can be linked for proper stereo operation (not just dual mono)
"All-button" or British mode
The ratio buttons are designed to be mutually exclusive, so that pressing one ratio button deselects the others. However, British engineers discovered it was possible to push all four buttons in at once, an unexpected use case that led to unintended behaviour, with a substantial increase of harmonic distortion. This became known as "All-button" mode or British mode,[4] and is popular enough to be explicitly supported by modern clones of the 1176.

The way the 1176 sounds, and specifically, the way all-button mode sounds, is partially due to its being a program dependent compressor. The attack and release are program dependent, as is the ratio.

The 1176 will faithfully compress or limit at the selected ratio for transients, but the ratio will always increase a bit after the transient. To what degree is once again material dependent. This is true for any of the 1176's ratio settings, and is part of the 1176's sound.

But in all-button mode, a few more things are happening; the ratio goes to somewhere between 12:1 and 20:1, and the bias points change all over the circuit. As a result, the attack and release times change. This change in attack and release times and the compression curve that results is the main contributor to the all-button sound. This is what gives way to the trademark overdriven tone. The shape of the compression curve changes dramatically in all-button. Where 4:1 is a gentle slope, all-button is more like severe plateau! Furthermore, in all-button mode there is a lag time on the attack of initial transients. This strange phenomenon might be described as a "reverse look-ahead".

— Will Shanks[5]
Revisions
The 1176 underwent a number of revisions; one notable change in the early revisions was the addition of Brad Plunkett's circuitry, which reduced noise by 6 dB and redistributed the noise spectrum, producing even more noise reduction in the sensitive mid-range; linearity was also increased by reducing harmonic distortion. These revisions, easily distinguishable for their solid black face panel, were labelled 1176LN.[6]

Revisions D and E are reputed to sound the best.[1]

Revision	Design date	Serial numbers	Notes
Revision A	June 20, 1967	101–125	The original design by Bill Putnam; it had a brushed aluminium faceplate with a blue meter section. The output transformer was the UA-5002.
Revision AB	November 20, 1967	126–216	Several resistor values changes in signal pre-amp stages improved stability and noise.
Revision B	Not indicated	217–1078	FETs in the signal pre-amp were replaced with bipolar transistors (2N3391A).
Revision C	January 9, 1970	1079–1238	Low noise ("LN") circuitry was added in the signal preamp, reducing DS voltage on the gain reduction FET and keeping the FET within its linear range. The FET feedback circuit was revised to minimize distortion. Program dependency behavior was re-tuned. The faceplate was changed to anodized black.
Revision D	Not indicated	1239–2331	This revision had no circuit changes, but the additional low-noise circuitry was incorporated into a new main circuit board. "UREI" branding was added to the original "Universal Audio" branding.
Revision E	Not indicated	2332–2611	New power transformer, now switchable between 110 V and 220 V.
Revision F	March 15, 1973	2612–7052	The output amplifier was changed from a class A to a push-pull (class AB) design, based on the 1109 preamplifier, providing more output drive. The output transformer was changed to the B11148 type (already used in UREI LA-3A). Metering circuit now uses an op-amp.
Revision G	Not indicated	7053–7651	The input transformer was removed and replaced with a differential amplifier. Program dependency behavior was removed.
Revision H	Not indicated	7652–8000+	The faceplate was changed to the original silver faceplate and included a red "Off" button. The only version with a blue "UREI" logo and without "Universal Audio" branding.
Re-issue	4 January 2000	101–1959	Reproduction based on C, D and E revisions; most resembling the E model, due to the use of the switchable power transformer.
1960–2946	Input attenuator and transformer changed, since the original Precision Electronics T-pad attenuator and 012 Magnetika transformer became unavailable.
2947–	Production were resumed with T-pad attenuators and 012 Magnetika input transformers.
Revision AE	June 4, 2008	001–500	The "Anniversary Edition", limited to 500 units worldwide and resembling the first revisions with a black faceplate and a blue painted stripe, introduced a more moderate compression ratio of 2:1, a slow 10 ms attack and reintroduced revision A program dependency.
Reputation
Mike Shipley says "The 1176 absolutely adds a bright character to a sound, and you can set the attack so it's got a nice bite to it. I usually use them on four to one, with quite a lot of gain reduction. I like how variable the attack and release is; there's a sound on the attack and release which I don't think you can get with any other compressor. I listen for how it affects the vocal, and depending on the song I set the attack or release—faster attack if I want a bit more bite. My preference is for the black face model, the 4000 series—I think the top end is especially clean."[4]

Jim Scott says "They have an equalizer kind of effect, adding a coloration that's bright and clear. Not only do they give you a little more impact from the compression, they also sort of clear things up; maybe a little bottom end gets squeezed out or maybe they are just sort of excitingly solid state or whatever they are. The big thing for me is the clarity, and the improvement in the top end."[4]

See also
UREI
LA-2A Leveling Amplifier
Empirical Labs Distressor
References
 Fuston, Lynn. "UA's Classic 1176 Compressor — A History". Universal Audio. Retrieved March 30, 2020.
 Hepworth-Sawyer, Russ; Hodgson, Jay; Paterson, Justin; Toulson, Rob (June 25, 2019). Innovation in Music: Performance, Production, Technology, and Business. Routledge. p. 204. ISBN 978-1-138-49821-1.
 "2008 TECnology Hall of Fame". Archived from the original on July 12, 2009. Retrieved April 28, 2009.
 "Model 1176LN – Solid-State Limiting Amplifier" (PDF). Universal Audio. 2009. Retrieved March 31, 2020.
 "Universal Audio". Archived from the original on August 29, 2008. Retrieved April 30, 2009.
 Shanks, Will. "1176 and LA-2A Hardware Revision History". Universal Audio. Retrieved March 30, 2020.



The 1176 sound refers to the distinctive compression characteristics produced by the Universal Audio 1176 Limiting Amplifier, a legendary analog compressor introduced in the 1960s. Renowned for its fast attack times, smooth compression, and unique FET (Field-Effect Transistor) circuitry, the 1176 has become a staple in professional recording studios worldwide. Its ability to add warmth, punch, and clarity to vocals, instruments, and mixes has made it a go-to tool for engineers and producers across genres. The 1176 sound is often characterized by its subtle yet transformative effect, enhancing dynamics while imparting a rich, musical quality that has defined countless iconic recordings.


History of the 1176 Compressor: Origins, development, and evolution of the iconic 1176 limiter/compressor
The 1176 compressor, a cornerstone of audio engineering, emerged in the late 1960s as a revolutionary tool for dynamic control. Designed by Bill Putnam Sr. of Universal Audio, the 1176 was born out of the need for a fast, versatile, and musically responsive limiter/compressor. Its origins trace back to the evolving demands of recording studios, where engineers sought a device that could tame peaks without sacrificing the natural character of the sound. The 1176’s FET-based design, a departure from the tube-driven compressors of the era, offered unprecedented speed and consistency, making it an instant favorite among engineers.

What set the 1176 apart was its unique blend of technical innovation and artistic flexibility. Its four ratio settings (4:1, 8:1, 12:1, and 20:1) allowed for everything from subtle compression to aggressive limiting, while the “All Buttons In” mode introduced a distinctive, almost unpredictable distortion that became a signature sound. The attack and release controls, though simple in design, provided a level of precision that enabled engineers to shape transients and sustain with surgical accuracy. This combination of features made the 1176 a go-to tool for vocals, guitars, bass, and even entire mixes, earning it a permanent place in the studio arsenal.

The evolution of the 1176 reflects its enduring appeal and adaptability. Early revisions, such as the Rev A and Rev E models, introduced subtle changes in circuitry and component choices, each imparting a slightly different tonal character. The blue-stripe and blackface versions, for instance, are prized for their warmth and punch, while later models like the silverface 1176LN offered increased headroom and cleaner operation. Despite these variations, the core sound of the 1176—fast, aggressive, and rich in harmonic distortion—remained consistent, ensuring its relevance across decades of technological advancement.

Today, the 1176’s legacy extends beyond its hardware incarnations. Modern software emulations and hardware recreations strive to capture its essence, often incorporating additional features like sidechain filters or mix controls. However, the original’s simplicity and immediacy remain unmatched. For engineers seeking the authentic 1176 sound, experimenting with input and output levels, ratio settings, and attack/release times can yield vastly different results. For instance, driving the input hard while dialing back the output can add warmth and grit, while faster attack times can tighten up percussive elements.

In conclusion, the 1176 compressor’s history is a testament to its groundbreaking design and timeless appeal. From its inception as a problem-solving tool to its status as an iconic piece of studio gear, the 1176 has shaped the sound of countless recordings. Its evolution, marked by subtle refinements and enduring principles, underscores its role as both a technical achievement and an artistic instrument. Whether in its original form or through modern interpretations, the 1176 continues to define the essence of dynamic control in audio production.




Key Features of the 1176: Unique characteristics, controls, and technical specifications of the 1176
The 1176 compressor, a staple in audio engineering since the 1960s, is renowned for its distinctive sound and versatility. Its unique characteristics stem from a combination of its Class A solid-state design, the use of discrete FET circuitry, and the famous "all-button" mode. These elements contribute to its ability to add warmth, character, and a subtle distortion that enhances vocals, guitars, and drums alike. Unlike tube compressors, the 1176 delivers a faster attack and a more aggressive response, making it ideal for shaping transients while maintaining clarity.

One of the most defining features of the 1176 is its four ratio settings: 4:1, 8:1, 12:1, and 20:1. Each ratio offers a distinct flavor of compression, from gentle taming at 4:1 to extreme limiting at 20:1. The "all-button" mode, where multiple ratio buttons are engaged simultaneously, creates a unique, non-linear compression curve that adds a signature punch and grit. This mode is particularly sought after for its ability to make vocals sit prominently in a mix while retaining their natural dynamics.

The controls on the 1176 are straightforward yet powerful. The input knob drives the signal into the compressor, allowing for varying degrees of saturation and distortion. The output knob compensates for gain reduction, ensuring the signal remains balanced. The attack and release knobs offer further control over the compression's timing, with attack times as fast as 20 microseconds. This precision enables engineers to fine-tune the compressor's response to suit the source material, whether it’s a sharp snare hit or a smooth vocal performance.

Technically, the 1176 operates within a frequency response range of 20 Hz to 20 kHz, ensuring transparency across the audible spectrum. Its signal-to-noise ratio exceeds 70 dB, minimizing unwanted noise. The unit’s gain reduction meter provides visual feedback, helping engineers monitor compression levels accurately. Modern iterations, such as the Universal Audio 1176 revisions, maintain the original’s sonic integrity while adding features like true bypass and stereo linking for enhanced usability in contemporary studios.

To maximize the 1176’s potential, experiment with its controls in context. For vocals, start with a 4:1 ratio, a moderate attack (500 microseconds), and a release around 50 milliseconds. Gradually increase the input gain to add harmonic richness without over-compressing. For drums, try the "all-button" mode with a fast attack to control transients while adding glue and cohesion. Remember, the 1176’s magic lies in its ability to enhance, not overpower—use it judiciously to preserve the source’s character while adding its iconic warmth and presence.

Understanding Echolalia: What It Sounds Like and How It Manifests
You may want to see also


Applications in Music Production: How the 1176 is used in recording, mixing, and mastering
The 1176 compressor, a staple in music production since the 1960s, is renowned for its distinctive sound and versatility. Its aggressive compression and fast attack times make it a go-to tool for shaping dynamics in recording, mixing, and mastering. Understanding its applications requires a deep dive into how its unique characteristics—such as its Class A circuitry, FET-based design, and program-dependent release times—interact with different audio sources and production stages.

Recording: Capturing Dynamics with Precision

In the recording phase, the 1176 excels at taming unruly performances while preserving their energy. For vocals, setting the attack to its fastest setting (20 microseconds) and using a moderate ratio (4:1) can smooth out plosives and inconsistent levels without sacrificing clarity. On drums, particularly snare or kick, a higher ratio (8:1 or above) with a slower attack (500 microseconds) adds punch and control, ensuring transients remain intact. A practical tip: engage the "All Buttons In" (ABI) mode for extreme compression, which creates a signature "crushed" sound often heard in rock and pop recordings. This technique works best when applied subtly, as over-compression can flatten the performance.

Mixing: Sculpting Tone and Balance

During mixing, the 1176 becomes a tonal shaper rather than just a dynamic controller. On bass guitar, a 4:1 ratio with a medium attack (100 microseconds) can tighten the low end while adding a subtle harmonic distortion that enhances warmth. For parallel compression on drums, blend a heavily compressed (12:1 ratio, fast attack) 1176 signal with the dry track to increase glue and impact without losing detail. Caution: avoid overloading the input gain, as the 1176’s distortion can become harsh if pushed too hard. Instead, aim for 3-6 dB of gain reduction to retain transparency while adding character.

Mastering: Subtle Enhancement, Not Overhaul

In mastering, the 1176 is used sparingly to add final touches rather than drastic changes. A 2:1 ratio with a slow attack and auto-release can gently control peaks while maintaining the mix’s natural dynamics. For stereo bus compression, use minimal gain reduction (1-2 dB) to add cohesion without compromising stereo width. A key takeaway: mastering is not the time for aggressive 1176 settings; its role here is to refine, not redefine.

Comparative Advantage: Why the 1176 Stands Out

What sets the 1176 apart in these applications is its ability to balance control and character. Unlike digital compressors, its FET design introduces harmonic distortion that adds depth and presence, making it ideal for adding "vibe" to tracks. For instance, while the LA-2A offers smooth optical compression, the 1176’s faster response and more aggressive nature make it better suited for sources needing tighter control. Its program-dependent release ensures it adapts to the material, providing a natural feel even under heavy compression.

Practical Tips for Optimal Use

To maximize the 1176’s potential, experiment with input and output levels to find the sweet spot between compression and distortion. For vocals, start with a 4:1 ratio and adjust the attack to taste. On instruments, use higher ratios sparingly to avoid losing dynamics. Always trust your ears: if it sounds good, it is good. Finally, consider using the 1176 in conjunction with other compressors for layered control—for example, pairing it with an optical compressor for a blend of aggression and smoothness.

By understanding its strengths and limitations, the 1176 can become an indispensable tool in your production arsenal, adding character and control at every stage of the process.

Inclusive Leadership in Action: The Language of Empathy and Empowerment


soundcy

Classic 1176 Sound Signature: Distinctive tonal qualities and compression style of the 1176
The 1176 sound is characterized by its aggressive yet musical compression, a hallmark of the Universal Audio 1176 Limiting Amplifier. This iconic hardware compressor, introduced in the late 1960s, has become a staple in professional recording studios for its ability to shape dynamics while imparting a distinctive tonal color. Its sound signature is not just about reducing volume; it’s about adding character, warmth, and a subtle distortion that enhances vocals, guitars, and drums alike. Understanding this signature requires dissecting its tonal qualities and compression style, which have influenced decades of music production.

At the heart of the 1176’s sound is its Class A solid-state design, which delivers a clean yet slightly gritty edge. When pushed hard, the 1176 introduces harmonic distortion, particularly in the upper midrange, that adds presence and bite to the source material. This is especially noticeable on vocals, where the compressor can make a performance sound more forward and intimate. For example, setting the input knob high (around +20 dB) while dialing in fast attack (1–2 ms) and release (20–80 ms) times creates the classic “all buttons in” mode, which saturates the signal with a punchy, aggressive character. This technique is often used on snare drums to add crack and sustain without losing impact.

The 1176’s compression style is equally distinctive, thanks to its FET (Field Effect Transistor) circuitry and unique ratio settings. Unlike modern digital compressors, the 1176’s ratios (4:1, 8:1, 12:1, and 20:1) interact with the input and output controls in a nonlinear way, creating a dynamic response that feels alive. For instance, applying a 4:1 ratio with moderate threshold settings can gently tame peaks on acoustic guitars, preserving their natural decay while adding a glue-like cohesion. Conversely, a 20:1 ratio acts almost like a limiter, perfect for controlling erratic transients in percussion or bass without sacrificing the instrument’s body.

To achieve the classic 1176 sound, start by experimenting with input and output levels. A common technique is to drive the input gain while reducing the output to balance compression and distortion. For vocals, try setting the threshold around -10 dB, attack at 20 ms, and release at 50 ms, adjusting to taste. On bass, a faster attack (1–2 ms) and higher ratio (12:1 or 20:1) can tighten the low end while adding a subtle growl. Remember, the 1176’s magic lies in its ability to enhance, not just control, so avoid over-compressing unless intentional distortion is the goal.

In comparison to other compressors, the 1176 stands out for its immediacy and color. While optical compressors like the LA-2A offer smooth, slow-acting compression, the 1176’s FET design provides faster response times and a more aggressive bite. This makes it ideal for sources that benefit from quick dynamic shaping, such as rock vocals or snare drums. However, its tonal qualities can also be too much for delicate acoustic instruments, where a more transparent compressor might be preferable. The key is knowing when to deploy the 1176’s signature sound—whether to add grit, glue, or sheer presence to a mix.

Do Vowels Represent One Sound? Exploring Phonetic Complexity and Variations
You may want to see also




soundcy

Modern Alternatives to the 1176: Hardware and software emulations replicating the 1176 sound
The 1176 sound, characterized by its fast attack, punchy compression, and subtle harmonic distortion, has been a staple in audio engineering for decades. However, the original hardware units are increasingly rare and expensive, driving the demand for modern alternatives. Both hardware and software emulations now offer accessible ways to replicate this iconic sound, each with its own strengths and trade-offs.

Hardware Emulations: Precision and Authenticity

Modern hardware emulations of the 1176, such as the Universal Audio 1176 Revision A/E/F reissues, aim to recreate the exact circuitry and component behavior of the original units. These devices use discrete Class A circuitry and transformers to deliver the warmth and aggression that made the 1176 legendary. For instance, the UA reissue allows users to switch between different revisions, each with unique sonic characteristics—Rev A for aggressive limiting, Rev E for balanced compression, and Rev F for cleaner response. These units are ideal for professionals seeking the authentic 1176 experience but come with a premium price tag, often exceeding $2,000. For those on a budget, alternatives like the Warm Audio WA76 offer a more affordable entry point, sacrificing some nuance but retaining the core 1176 sound.

Software Emulations: Versatility and Affordability

Software plugins provide a cost-effective and versatile solution for achieving the 1176 sound. Plugins like the Waves CLA-76, modeled after the Rev E and Rev D, offer precise control over attack and release times, often with additional features like mid-side processing or mix controls. The Universal Audio 1176 plugin, part of their UAD platform, is highly regarded for its accuracy, though it requires a UAD DSP accelerator. For DAW-native options, the Softube FET Compressor and the Slate Digital FG-Grey stand out for their detailed modeling and low CPU usage. These plugins typically range from $50 to $300, making them accessible to home studios and professionals alike.

Practical Tips for Choosing an Alternative

When selecting a modern alternative, consider your workflow and needs. Hardware units excel in hybrid setups, where analog warmth can complement digital production. However, they require physical space and maintenance. Software plugins, on the other hand, offer instant recall and A/B comparison, essential for fast-paced production environments. For vocal tracking, a hardware unit’s tactile controls can inspire creativity, while plugins are ideal for mixing and mastering. Experiment with demo versions of plugins to find the one that best captures the 1176’s signature punch and character for your specific application.

The Takeaway: Balancing Tradition and Innovation



\While nothing fully replaces the original 1176, modern alternatives come remarkably close, each catering to different priorities. Hardware emulations preserve the analog magic but demand investment, while software plugins democratize access with added flexibility. Whether you’re a purist or a pragmatist, the 1176 sound remains within reach, ensuring its legacy endures in both classic and contemporary productions.1176 by Universal Audio: The Ultimate Guide to the Most Iconic Compressor Ever
July 8, 2025
The 1176 is a legendary compressor/limiter designed by Bill Putnam Sr. in 1967. It was one of the first fully solid-state compressors, and it marked a major turning point in audio production history. With its ultra-fast attack, musical harmonic distortion, and powerful sound-shaping capabilities, the 1176 has become a go-to tool in professional studios worldwide.

1176 Revisions and Versions: What’s the Difference?
Over the years, multiple revisions of the 1176 have been released. While the core design remains, each revision has unique tonal characteristics due to differences in components, transformers, and circuit paths:

• Rev A (“Blue Stripe”)
The very first version. Bright, aggressive, and very colorful. Beloved on vocals and drums for its character.

• Rev B/C/D/E
More stable circuitry, lower noise. Slightly cleaner than Rev A, but still with the 1176’s signature punch. Rev E is often considered the “classic” version.

• Rev F/G
Output stage changed from Class A to push-pull. Cleaner, more modern sound. Great when you want control without too much color.

• 1176LN (Low Noise)
Modern version with reduced background noise. The basis for many modern hardware and plugin emulations.

How Does the 1176 Work?
The 1176 is a FET (Field Effect Transistor) compressor, which gives it a tube-like warmth in a solid-state design. The FET circuit delivers musical saturation and fast response, ideal for controlling peaks while adding character.

Key Features:
Ratio: From 4:1 up to ∞:1

Attack: 20 microseconds to 800 microseconds — ultra-fast

Release: 50 milliseconds to 1.1 seconds

All-Buttons Mode (“British Mode”): When all ratio buttons are engaged, the 1176 becomes a wild beast — perfect for heavy pumping, distortion, and creative compression

How and When to Use the 1176 in Your Mix
The 1176 is incredibly versatile, but some applications are where it truly shines:

🔹 Vocals
Great for adding presence, energy, and forward punch

Use medium-fast attack and fast release for an “in-your-face” vocal

🔹 Drums (Especially Snare and Room Mics)
All-Buttons Mode on room mics is legendary for bringing out ambience and smack

On snare, it adds crack and weight with aggressive character

🔹 Bass
Excellent for leveling dynamics and adding punch

4:1 or 8:1 ratios work best for thick, stable low end

🔹 Electric Guitars
Adds harmonic grit and makes parts sit tight in the mix

Also useful to tame harsh peaks without dulling the tone

Best 1176 Plugin Emulations
There are many software versions of the 1176 available today. Some of the most accurate and popular include:

UAD 1176 Collection (Universal Audio): The official emulation, with meticulous modeling of various revisions

Waves CLA-76: Offers two modes (Bluey and Blacky), great sound for the price

Slate FG-116: Adds modern controls while staying true to the vintage vibe

IK Multimedia Black 76

Arturia Comp FET-76




The UREI/Universal Audio 1176 — Technical Deep Dive

Circuit theory, the FET gain-reduction math, sidechain behavior, and a revision-by-revision comparison. Written with audio-DSP modeling in mind.


1. What it actually is

The 1176 is a solid-state, FET-based peak limiter designed by Bill Putnam Sr. at Universal Audio, first shipped in 1967. It descended from Putnam's tube limiters (the UA 175 and 176) but replaced the vacuum-tube gain element with a Field Effect Transistor used as a voltage-variable resistor (VVR). That single design move is the whole story: it gave the unit its trademark 20 µs attack — orders of magnitude faster than any vari-mu or opto design of the era — and its distinctive distortion signature.

Three architectural facts define its behavior, and you should hold all three in your head before touching the math:


It's a feedback (feedback-loop) compressor. The sidechain senses the signal after gain reduction, not before. This makes the compression curve soft-knee and strongly program-dependent.
Gain reduction happens by attenuation, not amplification. The FET forms the shunt (bottom) leg of a voltage divider ahead of a fixed-gain preamp. The FET never "amplifies" — it throws signal away, and a fixed ~45 dB amplifier behind it makes up the difference.
There is no threshold knob and no makeup-gain-by-threshold. Input sets both the drive into the divider and the effective threshold. Ratio buttons additionally shift the threshold.



2. Signal path (block level)

                                     ┌─────────── sidechain tap (post-GR) ──────────┐
                                     │                                              │
IN ─► Input atten ─► Input xfmr ─► [ R5 / FET divider ] ─► Preamp (~26 dB) ─┬─► Output atten ─► Output amp (Class A) ─► Output xfmr ─► OUT
       (Input pot)                   (gain reduction)                       │        (Output pot)
                                          ▲                                 │
                                          │ gate (control V)                ▼
                                  Gain-Reduction Control  ◄──── Ratio-button bias ──── full-wave rectifier ◄── phase splitters ◄── (tapped here)
                                  (attack R55, release R56, smoothing C22)

Stage by stage:


Input section — a passive adjustable attenuator (the Input pot) followed by an input transformer. Its job is to knock the level down so the FET divider isn't overdriven, and to set how hard you hit the compressor. This transformer-coupled front end was used on revisions A through F.
Gain-reduction divider — series resistor R5 on top, FET (Q1) on the bottom. The audio is applied across the divider; the FET's drain-source resistance sets the division ratio, hence the gain.
Preamp — a high-gain solid-state stage (~26 dB) that restores level after the divider. The sidechain is tapped from the output of this preamp, just before the Output pot (R23) — that's what makes it a feedback design.
Output amplifier — a Darlington pair driving a Class A stage built around a 2N3053, essentially the UA 1108 preamp output stage. It drives a custom Putnam output transformer whose extra feedback windings are part of the amplifier's stabilization network, not just impedance matching. (From rev F onward this became a push-pull Class AB stage borrowed from the 1109.)
Gain-reduction control (sidechain) — phase splitters, a full-wave rectifier, an RC smoothing/timing network, and a ratio-dependent bias. Output is a DC control voltage to the FET gate.



3. The heart of it: FET as a voltage-variable resistor

3.1 Why a FET works as a resistor

A JFET has three operating regions. Up in the saturation (pinch-off) region it behaves as a voltage-controlled current source — that's how you'd use it as an amplifier. But down in the ohmic / triode / linear region, where the drain-source voltage V_DS is small (roughly < 100 mV), the channel behaves like a resistor whose value the gate voltage sets.

For an N-channel depletion-mode JFET in the ohmic region, the drain current is:

I_D = K · [ (V_GS − V_P)·V_DS − V_DS²/2 ]

where:


V_GS = gate-source voltage (the control voltage)
V_DS = drain-source voltage (the audio across the FET)
V_P = pinch-off voltage (negative for an N-channel JFET, a fixed device parameter, e.g. −4 V)
K = transconductance constant ≈ 2·I_DSS / V_P²


3.2 Deriving the channel resistance

Channel conductance is the derivative of I_D with respect to V_DS:

g_ds = ∂I_D/∂V_DS = K · [ (V_GS − V_P) − V_DS ]

and the channel resistance is its reciprocal:

r_ds = 1 / g_ds = 1 / { K · [ (V_GS − V_P) − V_DS ] }

For small signals (V_DS → 0) this collapses to the clean, useful form:

r_ds ≈ 1 / [ K · (V_GS − V_P) ]

or, normalized to the on-resistance at V_GS = 0:

r_ds(V_GS) ≈ r_ds(on) / ( 1 − V_GS/V_P )

Read this physically:


When V_GS = 0, the channel is wide open → r_ds is at its minimum (r_ds(on), often a few hundred ohms).
As V_GS moves toward V_P, the denominator shrinks → r_ds climbs.
At V_GS = V_P, the channel pinches off → r_ds → ∞.


So a relatively small DC control-voltage swing sweeps the FET from a few hundred ohms to effectively open-circuit. That's an enormous resistance ratio from a tiny control range, which is exactly why the attack can be microseconds-fast.

3.3 From resistance to gain

R5 and the FET form a voltage divider, FET on the bottom:

Gain = r_ds / (R5 + r_ds)

When the control circuit drives the FET resistance down, the divider throws away more signal → gain drops → limiting. Larger input → larger detected level → control voltage moves the FET toward lower resistance → more gain reduction. That feedback loop is the limiting action.


Sign-convention warning for modeling: different sources describe the gate drive polarity relative to either ground or the resting bias, so you'll see both "more positive = less resistance" and "drive negative for GR." Don't anchor on the absolute sign — anchor on resistance falls → gain falls → that's gain reduction. Match polarity to your own bias reference.




4. Where the "1176 sound" comes from (the distortion math)

This is the part that matters most if you're modeling it.

Look again at the triode equation. The term that breaks linearity is − V_DS²/2. Because the drain current — and therefore the channel resistance — depends on the square of the audio voltage across the FET, the resistance is modulated by the very signal passing through it. A squared term in the transfer characteristic generates predominantly 2nd-harmonic (even-order) distortion. That asymmetric, even-harmonic character is a big part of the 1176's "warm / present / bright" coloration.

4.1 The linearization trick (and what "LN" really is)

There's a classic fix, and the 1176 uses a version of it. If you feed half of the drain-source voltage back into the gate, i.e.

V_GS = V_control + V_DS/2

then substitute into the triode equation:

I_D = K · [ (V_control + V_DS/2 − V_P)·V_DS − V_DS²/2 ]
    = K · [ (V_control − V_P)·V_DS + V_DS²/2 − V_DS²/2 ]
    = K · (V_control − V_P)·V_DS

The V_DS²/2 terms cancel, leaving a current that's linear in V_DS. In practice this is done with two equal resistors injecting a fraction of the audio across the FET back onto the gate. This is the standard FET-VVR linearization, and Brad Plunkett's "LN" (Low Noise) circuit — introduced at rev C — is fundamentally about keeping the FET inside its linear window so its noise and distortion contribution drop. The cancellation is only first-order (it never fully zeroes higher orders, and it drifts with bias), which is exactly why even an LN unit still colors the signal. Model it as imperfect 2nd-harmonic cancellation, not a clean linearizer.

4.2 Practical modeling takeaways


The static nonlinearity is well captured by a waveshaper derived from the measured r_ds(V_gs, V_ds) surface, or from the triode equation with a part-specific V_P, I_DSS.
It's even-harmonic dominant at low/moderate GR, getting grittier (more odd content, IMD) as you slam it — your ADAA approach is the right tool for antialiasing this static curve.
Because detection is post-GR (feedback), do not model it as a feedforward static curve. Close the loop: control voltage depends on the already-reduced output, which softens the knee and makes the ratio bend with program material.
Both transformers matter. The input transformer adds low-end saturation and a slight HF character; the output transformer is inside the feedback network and contributes the bulk of the "iron." Your Jiles-Atherton transformer model is appropriate here, especially for the output stage.



5. The sidechain / detector in detail

From the UA service description, with original part designators:


The detector input is tapped from the preamp output, just before the Output pot (R23).
The Ratio push-buttons set how much of that signal reaches the sidechain (this sets ratio) and apply a bias to the rectifier diodes (this sets threshold per ratio).
Q7 is a phase inverter, followed by emitter-follower Q8, feeding diode CR3. A second inverter/follower pair, Q9/Q10, feeds CR2 with a signal 180° out of phase. CR2 + CR3 together form a full-wave rectifier.
The rectified output is smoothed by C22, producing a DC control voltage proportional to signal level.
R55 sets attack by controlling how fast C22 charges.
R56 sets release by controlling how fast C22 discharges.
That control voltage drives the FET gate.


So the timing law is essentially a one-pole RC on the rectified, ratio-biased level — but split into separate charge (attack) and discharge (release) paths, both program-dependent because the loop is closed around the post-GR signal.

Time constants and curve

ParameterRangeNotesAttack20 µs – 800 µsAmong the fastest of any analog compressorRelease50 ms – 1.1 sOne-pole-ish discharge of C22Knob directionBackwardsFully CCW = slowest; fully CW = fastest. Counterintuitive, and the printed numbers (1→7) increase as the times get shorterKneeSoftConsequence of feedback detection


6. Ratio, threshold, and "All-Button" mode

6.1 Ratios

Four buttons: 4:1, 8:1, 12:1, 20:1.


4:1 / 8:1 → compression.
12:1 / 20:1 → limiting (the 20:1 approaches brick-wall).
Higher ratios also raise the threshold. This is why you'll often see less gain-reduction on the meter when you jump from 4:1 to 12:1 at the same input — you have to re-balance Input/Output when you change ratio. It's not a bug, it's how the ratio-button bias network interacts with the detector.


6.2 All-Button / "British" mode

Press all four ratio buttons at once. Mechanically, each button switches a different bias/ratio resistor into the sidechain; engaging all of them puts a parallel combination into the network that the circuit was never trimmed for. The results:


Effective ratio lands somewhere between 12:1 and 20:1.
Bias points shift all over the circuit, which changes the attack and release times and moves the detector's operating point.
The detector lags and overshoots on initial transients — a "reverse look-ahead" lag — producing the famous aggressive, distorted, pumping tone. The meter often pins.


It's harmless to the unit and is a signature drum-room / parallel-bus / in-your-face-vocal trick.


"Dr. Pepper" setting, for reference: the old soda's "10, 2, and 4" slogan → Attack ~10 o'clock, Release ~2 o'clock, Ratio 4:1. A common, musical vocal starting point.




7. Class A output and the transformer

The pre-rev-F output stage is Class A: the active devices conduct over the entire signal cycle, so there's no crossover distortion (the device never switches off and back on across the zero crossing). The penalty is heat and current draw; the payoff is linearity and that smooth low-level behavior.

The output transformer is a custom Putnam design and is genuinely part of the amplifier, not a bolt-on. It carries extra feedback windings (a tube-era practice) that are wired into the output stage's feedback loop to stabilize it. Putnam reportedly spent a long time qualifying the handful of vendors who could wind it to spec. For modeling, treat the output stage + transformer as a coupled, feedback-linked nonlinear system, not a clean amp followed by an independent transformer.


8. Revision-by-revision comparison

There were at least 13 revisions over the unit's life. The big inflection points:

Rev~YearFaceplateGR / LNInput stageOutput stageNotesA1967Silver, blue meter strip ("blue stripe")FET VVR, no LNTransformer + attenuatorClass A (1108-style)The original. Higher noise, distinct aggressive top end. The famed "bluey."AB1967 (months later)Silver / blueNo LNTransformerClass AStability fixes, slightly lower noise.B1968Silver / blueNo LNTransformerClass AMinor preamp-circuit changes. (A/AB/B = the "blue stripe" family.)CSept 1970BlackFirst LN — Plunkett's low-noise circuit, built as a potted epoxy module bolted to a rev-B boardTransformerClass ABecomes the "1176LN." "LN" = Low Noise. Patent-secrecy is why it was a separate module.D (D1)early '70sBlackLN integrated onto the main PCB (module gone, easier to build)TransformerClass AOne of the two most revered.Eearly '70sBlackLN integratedTransformerClass AEssentially D + 220 V mains voltage selector + a 10 MΩ resistor across the ratio switch to kill "pops" when changing ratios. D/E are the most sought-after — the "vintage blackface" sound.F1973BlackLNTransformerPush-pull Class AB (1109-derived)Output stage redesigned for higher output current; meter driver changed from discrete to an op-amp. Cleaner, more drive, slightly less of the Class-A magic.Gmid '70sBlackLNDifferential op-amp (electronically balanced) — input transformer GONEClass ABLoses the transformer front end; considered "less vintage" by most.HlaterSilver again, blue UREI logoLNOp-amp inputClass ABLargely cosmetic vs. G — return to the silver face.

What the revisions mean sonically


Blue stripe (A/AB/B): the aggressive, slightly noisier, raw end of the family. Prized for rock, drums, attitude. UA's modern "Legacy 1176" / 1176AE-style models chase this.
Blackface LN (D/E): the classic. Transformer in and Class A out and integrated LN — the combination most engineers mean when they say "1176 sound." This is what UA's 2000 reissue and most plugins model.
F/G/H: louder and cleaner with more output headroom and current, but the Class AB output (F+) and especially the op-amp input (G+, no transformer) strip away vintage character. Useful, reliable, less "iron."


The "LN" suffix appears from rev C onward. Before that the unit was just "1176."


9. Published specifications (UA 1176LN reissue, representative of D/E)

SpecValueInput impedance600 Ω, bridged-T control (floating)Output load impedance600 Ω (floating)Frequency response20 Hz – 20 kHz, ±1 dBGain45 dB, ±1 dBTHD< 0.5% (50 Hz–15 kHz, with limiting, 1.1 s release); +22 dBm out at ≤0.5% THDSignal-to-noise> 81 dB (input at threshold of limiting, 30 Hz–18 kHz BW)Attack20 µs – 800 µsRelease50 ms – 1.1 sConnectionsXLR + original Jones barrier (terminal) stripStereoOptional, via 1176SA adapter (fastest attack doubles to 40 µs when linked)Power115 V / 230 V selectableSize / weight19" × 3.5" × 12.25" (2U) / 11 lb

Reference levels worth keeping handy: +4 dBm = 1.23 V, 0 dBm into 600 Ω = 0.775 V RMS, −10 dBV = 0.316 V.


10. Calibration trims (useful if you're reverse-engineering or matching a unit)


Zero Set (R71): with Input fully CCW and meter in GR, set meter to 0 dB.
"Q" Bias (R59): sets the compression threshold. It's tied to the specific GR FET (Q1) — if you swap the FET, you must re-trim. Procedure: R59 fully CCW, apply a sine, set Input/Output for 0 dB out, then bring R59 CW until output drops exactly 1.0 dB. This is essentially biasing the FET to the edge of its linear gain-reduction window — the operating point that defines threshold and distortion onset.
Meter Driver Null (R75): null the voltage across R74 for an accurate GR meter.


The fact that threshold is set by FET bias (Q-bias) rather than a front-panel control is the deep reason the 1176 has no threshold knob — the threshold is baked into the device's bias point and nudged by the ratio buttons.


11. Quick reference: how to read the 1176 for a model


Topology: feedback peak limiter, attenuation-based.
Gain element: N-channel JFET in the ohmic region, shunt leg of an R5 divider. Gain = r_ds/(R5+r_ds).
Control law: r_ds ≈ r_ds(on)/(1 − V_GS/V_P); gate driven by rectified, smoothed, ratio-biased post-GR level.
Color: even-harmonic from the V_DS²/2 triode term; partially cancelled by half-V_DS gate feedback (the LN idea); never fully linear.
Timing: charge/discharge of C22 via R55 (20–800 µs) / R56 (50 ms–1.1 s); knobs are reversed.
Ratio/threshold: coupled — higher ratio raises threshold; all-buttons = parallel bias network → ~12–20:1 + shifted bias = the trademark grind.
Iron: input transformer (A–F) + a feedback-integrated Class A output transformer (A–E). Both color the sound; the output transformer is inside the loop.
Revision to model: D/E for "the" sound, blue-stripe for aggression, F+ for cleaner/louder.



Primary technical source: Universal Audio 1176LN service/operating manual (history, circuit details, calibration, specs). FET ohmic-region equations from standard JFET device physics (triode-region drain-current model and VVR linearization).


The core FET math. Gain reduction is a voltage divider, Gain = r_ds/(R5 + r_ds), with the FET as the shunt leg. The triode-region channel resistance derives to r_ds ≈ r_ds(on)/(1 − V_GS/V_P), which is why a tiny control swing sweeps it from a few hundred ohms to open-circuit (hence the 20 µs attack).
Where the "sound" comes from. The −V_DS²/2 term in the triode drain-current equation modulates the channel resistance with the signal itself → even-harmonic distortion. The half-V_DS-to-gate feedback trick cancels that term to first order, and that cancellation is essentially what Plunkett's LN circuit is doing — imperfectly, which is why even LN units color. That's the thing to model as partial 2nd-harmonic cancellation, not a clean linearizer, and a good ADAA candidate.
Two things that trip up feedforward models: it's a feedback detector (senses post-GR, so soft-knee and program-dependent — close the loop), and ratio/threshold are coupled (higher ratio raises threshold; all-buttons drops a parallel bias network in that lands ~12–20:1 with shifted bias points).
Revision summary: A/AB/B = blue-stripe, no LN, Class A, aggressive. C = first LN (epoxy module). D/E = the revered blackface — transformer in + Class A out + integrated LN (what most plugins model). F = Class AB push-pull output. G = loses the input transformer for an op-amp front end. H = cosmetic silver face.
One caveat worth flagging: gate-drive polarity is described inconsistently across sources depending on their bias reference, so I anchored everything on "resistance falls → gain falls" rather than absolute sign — match the sign to your own bias node when you build it.

# The 1176 — Advanced Notes (Part 2)

*Device-level electronics, the feedback-loop math, the real distortion mechanisms, noise/speed science, and DSP-modeling implications. Companion to the main deep-dive.*

---

## A. Device-level electronics

### A.1 The actual gain-reduction FET (Q1)

The gain element was an **N-channel JFET**, originally a **factory-selected device carrying only a UREI house number (13-0027)**, long unobtanium. The standard modern substitute is the **2N5457 / 2N5458** (clones also use the **BF245**), and it has to be **selected/matched** — you can't just drop one in. Pinch-off for a 2N5457 in this circuit lands around **−0.7 V to −1.5 V**.

Crucially, **Q1 is matched to a second FET, Q11**, which mirrors Q1's action to **drive the gain-reduction meter**. So the GR meter isn't reading the audio — it's reading a replica of the control action through a twin FET. That's why a unit "won't work right" without the correct, matched pair, and why swapping Q1 forces a Q-bias recalibration.

### A.2 Resolving the gate-polarity question (definitively)

In Part 1 I flagged the sign convention as ambiguous across sources. The device data settles it:

- **At rest (no signal):** the gate sits **negative, near pinch-off** → `r_ds` is **high** → the FET shunts almost nothing → signal passes at ~unity. **No gain reduction.**
- **Under signal:** the sidechain drives the gate **toward 0 V (more positive)** → `r_ds` **falls** → the FET shunts more signal to ground → **gain reduction.**

So the control voltage moves the gate **from near-pinch-off toward zero** to compress. The **Q-bias trim (R59)** sets exactly *where* that resting point sits on the FET's curve — which is precisely why **Q-bias = threshold**, and why threshold is a property of the device's bias point rather than a front-panel knob.

### A.3 Topology: shunt-to-ground, not just "bottom of a divider"

The cleanest way to picture the original: a **series resistor (R5) feeds a node, and Q1 shunts that node to ground.** Output is taken at the node:

```
V_node / V_in = r_ds / (R5 + r_ds)
```

High `r_ds` (rest) → ratio ≈ 1 (full signal). Low `r_ds` (compressing) → most of the signal is dumped to ground → big attenuation. Functionally identical to "FET as bottom leg of a divider," but the **shunt-to-ground mental model** is the accurate one for the original circuit and explains the LN move (below) better.

### A.4 The transformers, by part number

- **Input transformer:** Peerless (later UTC), **600 Ω**, feeding a resistive attenuator. The spec sheet calls the Input control a **bridged-T (constant-impedance) attenuator** — meaning the input impedance stays ~600 Ω as you sweep Input, preserving the matched line. Used revs **A–F**; gone from rev G (op-amp input).
- **Output transformer:** **UA-5002 (later UA-5002A)**, **600 Ω**, with a **split secondary**, a **tertiary winding that provides negative feedback to the line-output stage**, and a **separate emitter winding**. It is wired *into* the output amplifier's feedback path, so it's an active part of the amp's stability and tone, not a passive output pad. This is the transformer UA spent enormous effort replicating for the reissue.

### A.5 The amplifier blocks

- **Preamp:** the **1108 circuit** — bipolar, with a **Darlington pair** — ~26 dB.
- **Output (rev A–E):** similar topology followed by a **2N3053 bipolar in Class A**, driving the UA-5002.
- **Rev F+:** push-pull **Class AB** output (from the 1109) for more current, plus an **op-amp meter driver** replacing the discrete one.

---

## B. The feedback-loop science (why the curve behaves the way it does)

The 1176 senses level **after** the gain reduction (feedback detection). That one fact explains the soft knee, the program-dependent ratio, and why it can't truly brick-wall. Here's the math.

### B.1 Steady-state ratio derivation

Work in dB. Let `L_in`, `L_out` be input/output levels, `G` the gain (dB, negative when reducing), `T` the threshold. The feedback control law says the reduction is proportional to how far the **output** sits above threshold:

```
G = −k · (L_out − T)      (for L_out > T)
```

and trivially:

```
L_out = L_in + G
```

Substitute and solve:

```
L_out = L_in − k(L_out − T)
L_out (1 + k) = L_in + kT
L_out = L_in/(1+k) + kT/(1+k)
```

The slope of the static curve above threshold is:

```
dL_out/dL_in = 1/(1+k)     →     effective ratio R = 1 + k
```

So the **ratio buttons set the sidechain loop gain `k`**:

| Front-panel ratio | k |
|---|---|
| 4:1 | 3 |
| 8:1 | 7 |
| 12:1 | 11 |
| 20:1 | 19 |
| ∞:1 (brick wall) | k → ∞ |

### B.2 Why this gives a soft knee and program-dependent ratio

Two consequences fall out of the loop:

1. **No true brick wall.** Infinite ratio needs infinite loop gain. A real feedback loop has finite `k`, so the hardest setting is "20:1," not ∞. The approach to limiting is inherently gentle — a **soft knee** is *structural*, not a deliberate detector shaping choice.

2. **`k` isn't constant.** The FET's gain law `f(·)` is nonlinear, so the effective loop gain changes with operating point. Near threshold `k` is small (gentle), deeper in it grows — that *is* the soft knee. And because the loop has finite attack/release, during a transient it **hasn't settled**, so the instantaneous ratio differs from the steady-state value. The documented behavior — *"the ratio is faithful on the transient, then creeps upward after it"* — is the loop continuing to chase the post-GR level. This program-dependent ratio creep is a core part of the sound, present at every ratio, and exaggerated in all-button mode where the bias points shift mid-event.

**Modeling consequence:** do not implement a static feedforward curve. Implement the implicit relationship `L_out = L_in − k(L_out − T)` as an actual closed loop (one-sample delay in the detector path, or solve iteratively), with `k` derived from the nonlinear FET law. Feedforward models miss the knee shape and the ratio creep.

---

## C. The distortion science (this is the part that defines the tone)

There are **three distinct distortion mechanisms** in an 1176, and they live in different places.

### C.1 Static FET nonlinearity → even harmonics

From the triode equation (Part 1), channel current carries a **`−V_DS²/2`** term. Because resistance is modulated by the square of the audio across the FET, you get predominantly **2nd-harmonic / even-order** distortion. This is the always-present "warmth," strongest when you drive the FET hard (high Input).

### C.2 What "LN" actually does to distortion

Brad Plunkett's Low-Noise circuit (rev C+) works by **reducing the drain-source voltage swing across the gain-reduction FET**, keeping it inside its linear window. Smaller `V_DS` → the `V_DS²/2` term shrinks faster than the linear term → **less 2nd-harmonic distortion and less noise** at once. A **Q-bias pot in the FET's feedback path** was added at the same time to trim residual distortion. So "LN" is simultaneously a **noise** fix and a **linearity** fix — same root cause (FET operating range), two symptoms. It never fully linearizes, which is why LN units still color.

### C.3 The low-frequency grit: rectifier-ripple feedthrough (the big one)

This is the mechanism most people get wrong, and it's the scientific reason **all-button mode and fastest attack/release sound dirtiest on bass and kick.**

The detector is a **full-wave rectifier**, so its output ripples at **2× the signal frequency**. Capacitor C22 (with the attack/release resistors) smooths that ripple into a DC control voltage. Whether it smooths *enough* depends on the ratio of the time constant to the ripple period:

- **High audio frequencies:** ripple period ≪ attack/release time constant → control voltage is smooth → clean, well-behaved gain reduction.
- **Low frequencies + fast attack/release:** ripple period **approaches** the time constant → the control voltage develops residual ripple **at 2× the signal frequency** → the **gain itself is modulated at 2f** → that gain modulation multiplies the audio, producing **harmonics and intermodulation sidebands**.

In other words, at low frequencies with fast settings, the compressor partially tracks the waveform **cycle-by-cycle** instead of the envelope, and that periodic gain wobble *is* the distortion. It scales with how low the frequency is — exactly matching the observation that **perceived distortion increases at lower frequencies** and that the effect is most dramatic on kick/bass in all-button mode (fastest, most loop gain, most bias shift).

**Modeling consequence:** if you over-smooth the detector (big control-side lowpass for "stability"), you **delete the LF grit**. Preserve the rectifier ripple through a faithful attack/release network and let it modulate the gain. This single choice separates a flat-sounding 1176 model from a convincing one.

### C.4 Transformer harmonics

Independent of the FET: the **input transformer** saturates at low frequencies / high level → adds **odd (3rd) harmonic** down low. The **output transformer** sits inside the feedback loop, so the loop partially corrects its nonlinearity, but core hysteresis still imprints character (Jiles-Atherton-style behavior). At high output levels and low frequencies the iron contributes its own compression-like saturation on top of the FET action.

---

## D. Noise and speed science

### D.1 Why 20 µs attack was achievable (and unprecedented in 1967)

The control terminal is a **JFET gate — essentially zero DC current, very high impedance.** There's no mechanical element, no photoconductor lag (unlike the LA-2A's opto cell, which takes milliseconds), no tube transconductance settling. The control voltage can slew almost as fast as the sidechain charge path (R55/C22) and rectifier allow. That's why the 1176 was the **first compressor with a microsecond-class attack** and remains among the fastest analog designs.

### D.2 Noise sources and why LN mattered

The gain-reduction node sits **in front of ~45 dB of make-up gain**, so *anything* noisy at that node gets amplified hard. Contributors: **Johnson (thermal) noise of the resistive divider**, **thermal noise of the FET channel resistance**, and **preamp input noise**. The original (non-LN) revs were audibly noisy partly because the FET was run over a wider, less optimal range. By constraining the FET's operating window, the LN circuit lowered both its noise contribution and its distortion — hence the spec **S/N > 81 dB** on LN units.

---

## E. Topology comparison (where the 1176 sits scientifically)

| Design | Gain element | Detection | Knee / ratio | Speed | Character |
|---|---|---|---|---|---|
| **1176 (FET)** | JFET as VVR (shunt) | **Feedback** (post-GR) | Soft, program-dependent, no true ∞:1 | **20 µs** attack | Bright, even-harmonic, gritty when pushed |
| **LA-2A (opto)** | Photocell/EL panel | Feedback | Very soft, frequency-dependent | Slow (ms), 2-stage release | Smooth, "rubbery," self-leveling |
| **Vari-mu (tube, 175/176, Fairchild)** | Tube transconductance | Feedback | Soft, increasing-ratio | Medium | Thick, level-dependent ratio |
| **VCA (dbx 160, SSL)** | Transconductance VCA | Usually **feedforward** | Precise, hard knee possible | Fast | Clean, accurate, can brick-wall / look-ahead |

The 1176's defining combination is **FET speed + feedback softness + transformer iron + Class A**. Feedforward VCA designs can be more precise and can do true brick-wall/look-ahead limiting, but they don't produce the 1176's self-correcting, program-dependent, even-harmonic behavior.

---

## F. DSP / modeling implications (practical checklist)

If you're building a model:

1. **Close the loop.** The detector senses the *output*. Implement the implicit `y = f(y)·x` with a unit-delay in the sidechain (or a Newton/fixed-point solve per sample). Don't fake it with a feedforward curve — you'll lose the knee and the ratio creep.
2. **Derive ratio from loop gain `k = R − 1`,** and let `k` vary with operating point via the nonlinear FET law (that *is* your soft knee, for free).
3. **Model the FET as a `r_ds(V_gs)` shaper** (triode equation or a measured/curve-fit table), gate biased near pinch-off at rest, driven toward 0 for GR. Keep the partial-`V_DS/2`-feedback linearization so even-harmonic cancellation is **imperfect**, not perfect.
4. **Preserve rectifier ripple.** Full-wave rectify, then apply the split attack (charge) / release (discharge) one-pole. Do **not** over-smooth — the residual 2f ripple feeding back into the gain is the LF distortion (Section C.3).
5. **Oversample.** A 20 µs attack plus gain modulation generates real HF content; run the control loop and the FET shaper at high rate (or oversample the whole thing) and **antialias the static nonlinearity (ADAA)**.
6. **Two transformers, two jobs.** Input: LF saturation / mild HF tilt (your Jiles-Atherton model). Output: nonlinearity inside the loop + emitter/feedback windings — model it coupled to the output stage, not as a tacked-on saturator.
7. **All-button = parallel bias network.** Model the ratio buttons as bias resistors into the sidechain; "all in" = the parallel combination → effective ratio ~12–20:1 **plus** shifted bias points that move attack/release and the FET operating point. The dirt is emergent from those shifts, not a separate "distortion" stage.

---

## G. Extra factual nuggets

- **Serial-number revision map (blue stripe era):** rev A — first built **June 20, 1967**, SN **101–125**; rev AB — Nov 1967, SN **126–216**; rev B — Nov 1967 to Jan 1970, SN **217–1078**. Roughly **9 major revisions** total (A, A/B, B, C, D, E, F, G, H), and about **one-third of all vintage units are the op-amp-input G/H** type.
- **Meter swap:** at rev C the meter changed from a **Weston** to a **Modutec** (with two overhead bulbs), alongside the blackface redesign.
- **The 2000 UA reissue** was based primarily on revisions **C, D, and E** (the blackface LN family).
- **Anniversary edition (1176AE / 40th):** the only stock 1176 with a different ratio set — **2:1, 4:1, 8:1, 20:1** (the 2:1 in place of 12:1), giving a gentler option than any classic unit.
- **Multi-button "secret" ratios:** besides all-four, engineers use 2- and 3-button combinations (e.g. 4+8, 8+12, 4+8+12) for intermediate ratio/bias blends — each combination drops a different parallel bias network in.
- **Bypass-as-color:** with Attack fully CCW (OFF), there's no gain reduction but signal still runs through the input/output transformers and gains their character — and the unit can be used as a **straight ~45 dB line amp**.

---

*Sources: UA 1176LN service manual; device-level details and revision/serial history cross-referenced from Vintage King's 1176 technical writeup, the 1176 Peak Limiter encyclopedia entry, and DIY build documentation (Hairball/MNATS, AXT Systems). FET ohmic-region and feedback-loop equations from standard device physics and control theory.*

# Modeling the 1176 in Software — DSP Masterclass (Part 3)

*The deepest layer: full derivations, nonlinear circuit modeling, antiderivative antialiasing, oversampling filter design, zero-latency architecture, and production-grade JUCE/C++. Companion to Parts 1–2.*

> Notation: lowercase `x[n]` = discrete signal, `f(·)` = a memoryless nonlinearity, `F1`,`F2` = its first/second antiderivatives, `fs` = sample rate, `T = 1/fs`. Angular frequency `ω = 2πf`.

---

## 1. The analog plant as equations

### 1.1 The whole device, compactly

The 1176 is a **time-varying gain** wrapped in a **feedback control loop**, with two static nonlinear coloration blocks (input iron, output iron + Class-A stage). Write the audio path as:

```
y(t) = A · g(t) · Hin(x(t)) ,  then  y(t) ← Hout(y(t))
```

- `x(t)` input, `A` ≈ fixed make-up gain (~10^(45/20) split across pre/post),
- `g(t)` ∈ (0,1] the FET divider gain (the dynamic element),
- `Hin`, `Hout` the transformer/amp saturation shapers,
- and `g(t)` is set by a sidechain that senses `y(t)` (feedback).

Everything hard about modeling this is in two places: **the FET law `g`** and **the feedback loop that drives it.**

### 1.2 The FET divider as a nonlinear, time-varying gain

From Part 1, the FET in the ohmic region:

```
I_D = K[(V_GS − V_P)V_DS − V_DS²/2],   K = 2·I_DSS / V_P²
```

It sits as a shunt; the node-to-ground voltage **is** `V_DS`. The divider gain is

```
g = r_ds / (R5 + r_ds),     r_ds = 1 / { K[(V_GS − V_P) − V_DS] }
```

The control voltage enters through `V_GS`; the audio enters through `V_DS` — and because `r_ds` depends on `V_DS`, the gain is signal-dependent. **That signal dependence is the distortion.** Two control inputs, one of them (`V_DS`) being the audio itself, is the entire reason this thing colors.

### 1.3 Harmonic generation, derived

Let the control set a quiescent conductance `G0 = K(V_GS − V_P)`. Then

```
1/r_ds = G0 − K·V_DS
```

Expand the divider gain `g(V_DS)` as a Taylor series about `V_DS = 0`. Writing `r0 = 1/G0` and `a = K/G0` (a small signal-sensitivity coefficient):

```
g(V_DS) = g0 + g1·V_DS + g2·V_DS² + …
```

The leading correction term `g1·V_DS` multiplies the signal → because `V_DS ∝ signal`, you get a **squared term in the output ∝ signal²**, i.e. **2nd harmonic**, with amplitude set by `a = K/G0`. Deeper into gain reduction `G0` shrinks → `a` grows → distortion rises with GR. **This is the quantitative version of "more input, more grit."**

**The half-V_DS linearization** (Part 1/2): drive the gate with `V_GS = V_ctrl + V_DS/2`. Substituting cancels the `V_DS²/2` term in `I_D`, which **removes the `g1` (2nd-harmonic) coefficient to first order**, leaving the residual `g2·V_DS²` term → **predominantly 3rd harmonic**, much smaller. Real units cancel imperfectly (resistor tolerance, the `½` is only exact at one bias), so you keep a *reduced but nonzero* 2nd plus a 3rd. **Model it as a tunable, imperfect even-harmonic canceller**, not a clean linearizer.

### 1.4 The feedback loop, linearized

The loop is implicit: `g = h(y)`, `y = A·g·x`. Linearize around an operating point and work in dB (Part 2 result):

```
L_out = L_in/(1+k) + kT/(1+k),   ratio R = 1 + k
```

`k` = sidechain loop gain (ratio buttons). The nonlinearity of `h(·)` makes `k` vary with level → **soft knee for free**. The finite attack/release means during transients the loop hasn't converged → **ratio creep** (the steady-state `R` is only reached after settling). **Both are emergent from the loop; don't bolt them on.**

---

## 2. The sidechain, as a system of ODEs

### 2.1 Detector chain

```
post-GR signal  ──► full-wave rectify ──► ratio bias  ──► nonlinear 1-pole (attack/release) ──► gate driver ──► g
                     |w_d(t)| = |y(t)|        +b(R)            C22 charge/discharge
```

The smoothing capacitor obeys a **piecewise one-pole ODE** with different time constants for charge (attack) and discharge (release):

```
dV_c/dt = (|y| − V_c) / τ ,   τ = τ_att  if |y| > V_c   else  τ_rel
```

with `τ_att` set by R55 (20 µs–800 µs), `τ_rel` by R56 (50 ms–1.1 s). The knob curve is *reversed* (CW = faster) and roughly **logarithmic** in pot rotation — map knob 0..1 to `τ` exponentially.

### 2.2 The low-frequency grit, derived as AM sidebands

The full-wave rectifier output ripples at **2ω** (twice the audio frequency). The one-pole only partially smooths it, so the control retains a ripple component:

```
g(t) ≈ G0 + Gm·cos(2ωt)      (Gm grows as τ → ripple period at low f / fast settings)
```

Multiply by a tone `s(t) = A·cos(ωt)`:

```
y(t) = g(t)·s(t)
     = A·G0·cos(ωt) + A·Gm·cos(2ωt)cos(ωt)
     = A·G0·cos(ωt) + (A·Gm/2)[cos(ωt) + cos(3ωt)]
```

So the ripple injects energy at the **fundamental** and a **3rd harmonic** (and higher odd terms for richer ripple). `Gm` is large exactly when the smoothing time constant approaches the ripple period — i.e. **low frequency + fast attack/release** — which is the analytic explanation for "all-button on the kick = dirt." **Modeling rule: don't over-smooth the detector. The residual ripple IS the sound.** If you lowpass the control "to be safe," you delete the grit.

### 2.3 Discretizing the ballistics (do it right)

Naïve `y[n] = α·x[n] + (1−α)·y[n−1]` with `α = 1−e^(−T/τ)` is fine for the envelope, but for the FET law in the audio path use a **TPT / zero-delay-feedback** one-pole (Section 5.2) so cutoff stays accurate near Nyquist and there's no frequency-warping error.

---

## 3. Choosing a circuit-modeling strategy

| Approach | What it is | 1176 fit |
|---|---|---|
| **Behavioral / block** | Model each stage's I/O behavior (waveshapers + filters + envelope) | **Best ROI.** The FET divider, ballistics, and iron each map to one block. Fast, tunable, RT-cheap. |
| **State-space / nodal (MNA)** | Solve circuit node equations per sample, Newton for nonlinear nodes | High fidelity, high cost. Use for the FET node if you want phase-accurate `V_DS` feedback. |
| **Wave Digital Filters (WDF)** | Port-wise scattering, nonlinear root element | Elegant for the divider + one nonlinearity; the feedback detector breaks the tree (needs an R-type adaptor or iteration). |
| **K-method / DK-method** | Systematic state-space for nonlinear circuits | Great for the output stage + transformer if you want rigor. |

**Recommended hybrid:** behavioral for the loop and ballistics; a small **nonlinear nodal solve (Newton, 2–3 iterations)** only for the FET node where `V_DS` feeds back into `r_ds`; **Jiles-Atherton** (which you already have) for the transformers. This keeps CPU sane while capturing the parts that actually matter sonically.

### 3.1 The FET node as a 1-D Newton solve

The node voltage `v = V_DS` satisfies an implicit equation (KCL at the divider node): drive `u` through `R5`, FET conducts `I_D(v)` to ground:

```
F(v) = (u − v)/R5 − I_D(v; V_ctrl) = 0
```

Newton:

```
v ← v − F(v)/F'(v),   F'(v) = −1/R5 − dI_D/dv
dI_D/dv = K[(V_ctrl − V_P) − v]      (from the triode law)
```

2–3 iterations from last sample's `v` as the warm start converges to machine precision because the function is smooth and nearly linear. This gives you the **exact signal-dependent gain** including the `V_DS²` distortion, without a Taylor approximation.

---

## 4. Antiderivative Antialiasing (ADAA) — full math

A memoryless nonlinearity `f` broadens the spectrum; harmonics above Nyquist fold back as aliasing. ADAA convolves `f` with a short continuous-time kernel **before** sampling, which lowpasses the generated harmonics. It's the cheapest big win in nonlinear plugin design.

### 4.1 First-order ADAA

Replace `y[n] = f(x[n])` with the **average of `f` over the segment** from `x[n−1]` to `x[n]`:

```
y[n] = ( F1(x[n]) − F1(x[n−1]) ) / ( x[n] − x[n−1] ),    F1' = f
```

**Ill-conditioning:** when `x[n] ≈ x[n−1]` the denominator → 0 (catastrophic cancellation). Fall back to the midpoint:

```
if |x[n] − x[n−1]| < ε:   y[n] = f( (x[n] + x[n−1]) / 2 )
```

This is a **continuous-time convolution of `f` with a 1-sample box** = a `sinc`-shaped lowpass on the harmonics. It buys ~1 extra order of alias suppression — often equivalent to 2× oversampling, for far less CPU.

For `f = tanh`: `F1(x) = log(cosh(x))` (clean, no special functions).

### 4.2 Second-order ADAA

Convolve with a **triangular** kernel (box ⋆ box) — needs `F2` (`F2' = F1`) and two samples of memory. Canonical recursion (Parker/Esqueda/Bilbao 2016), with the three ill-conditioned branches handled:

```
calcD(x0, x1):
    if |x0 − x1| < ε:  return F1( (x0+x1)/2 )
    else:              return ( F2(x0) − F2(x1) ) / (x0 − x1)

process(x):
    d0 = calcD(x, x1)
    if |x − x2| < ε:
        xbar  = 0.5*(x + x2)
        delta = xbar − x1
        if |delta| < ε:  y = f( 0.5*(xbar + x1) )
        else:            y = (2/delta) * ( F1(xbar) + (F2(x1) − F2(xbar))/delta )
    else:
        y = (2/(x − x2)) * (d0 − d1)
    d1 = d0;  x2 = x1;  x1 = x;     // update state
    return y
```

2nd-order ADAA suppresses aliasing ~12 dB/octave better than 1st near the band edge; combined with **2× oversampling** it gets you to a near-inaudible alias floor for most drive levels.

### 4.3 ADAA's hard limits (read this before you ship)

- **ADAA is for *memoryless* nonlinearities only.** The 1176 FET in a feedback loop is *not* memoryless. **Apply ADAA to the static shaper components** — the transformer/Class-A saturation (`Hin`,`Hout`) and the static part of the FET curve — **not to the closed loop.** The loop's own lowpass (ballistics) already band-limits the control signal.
- ADAA introduces a tiny **bias/DC and ½-sample group delay**; usually negligible, but account for it in null tests.
- At very high drive, ADAA alone isn't enough — combine with oversampling (next).

---

## 5. Oversampling — design it, don't guess

### 5.1 Why and how much

A polynomial nonlinearity of order `N` generates harmonics up to `N×` the input frequency. To keep the `N`th harmonic of a top-octave tone below Nyquist you'd need `~N×` oversampling — impractical for high-order/`tanh`-like curves (effectively infinite order). So the real strategy is: **moderate oversampling × ADAA × gentle nonlinearities.** Targets for a world-class unit:

- **Tracking / mixing build:** 2×–4× OS + 1st/2nd-order ADAA, **min-phase IIR** halfband filters, alias floor ≤ −90…−100 dB.
- **Mastering build:** 4×–8× OS, **linear-phase FIR** halfband, alias floor ≤ −120 dB, latency reported to host.

### 5.2 Filter choices

- **Halfband FIR (polyphase):** for an exactly-2× stage, ~half the taps are zero → cheap. Linear phase (symmetric) → constant group delay = (N−1)/2 samples → **latency**. Use cascaded 2× stages (2→4→8) rather than one big filter.
- **Polyphase IIR allpass halfband (elliptic):** near-zero latency, minimum phase, very steep for few coefficients. The low-latency default. Slight phase nonlinearity (fine for a colorific compressor).
- **Transition band:** put the cutoff at ~`0.5·fs_base` with a guard; stopband attenuation = your alias-floor target. Don't make it steeper than needed — steeper = more taps = more latency/CPU.

### 5.3 The TPT one-pole (use everywhere you filter)

Zavalishin's trapezoidal, zero-delay-feedback one-pole — no bilinear cutoff warping error, stable, and it's the building block for the ballistics smoother and any tone shaping:

```
g = tan(π · fc / fs);            // prewarped
G = g / (1 + g);
v = (x − s) * G;                 // s = state
y = v + s;
s = y + v;                       // update
```

Swap `fc` for attack/release by recomputing `g` per branch (cache both).

---

## 6. Latency: avoid it by design

**The 1176 is a feedback compressor — it needs no lookahead.** That's a real architectural gift: unlike a feedforward brickwall limiter (which must delay the audio to peek ahead), your 1176 model can be **truly zero-latency at the base rate.** The only latency sources you might introduce:

1. **Linear-phase OS filters** → symmetric FIR delay. *Avoid* by using **min-phase IIR polyphase halfband** OS for the tracking build; reserve linear-phase for an opt-in mastering mode and **report it** via `setLatencySamples()`.
2. **Lookahead** → you don't need any. Don't add it.
3. **FFT/partitioned processing** → not needed here.
4. **The control-loop unit delay.** A feedback detector has a delay-free loop (`g[n]` needs `y[n]` needs `g[n]`). Break it with **one sample of delay in the control path only** (use `y[n−1]` to compute `g[n]`). This is inaudible (the control moves slowly vs audio) and adds **zero audio latency** — the audio sample is never delayed, only the gain estimate lags by one sample. Do **not** delay the audio to "fix" it.

**Net:** a correctly-built 1176 reports **0 samples** latency in the low-latency mode. Make latency a *mode*, never an accident.

---

## 7. World-class plugin engineering (JUCE / C++)

### 7.1 Real-time audio thread commandments

On the audio thread, **never**: allocate/free, lock a mutex, throw, log/print, call into the OS, or touch the GUI. **Always**: preallocate in `prepareToPlay`, pass parameters via lock-free atomics, kill denormals, and keep the block loop branch-light.

```cpp
void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
{
    juce::ScopedNoDenormals noDenormals;          // FTZ/DAZ — avoid denormal stalls
    // ... no allocations below this line ...
}
```

### 7.2 Parameters: APVTS + smoothing

```cpp
// In constructor: APVTS with ranged params; cache atomic pointers once.
inputGain  = apvts.getRawParameterValue ("input");
ratioParam = apvts.getRawParameterValue ("ratio");

// In prepareToPlay: set smoothing ramp time, not per-sample alloc.
smoothedInput.reset (sampleRate, 0.02);           // 20 ms ramp, click-free
```

Read atomics once per block, push into `juce::SmoothedValue`, advance per sample. Never read the host parameter object per sample.

### 7.3 An ADAA tanh saturator (drop-in, header-only)

```cpp
#pragma once
#include <cmath>

// 1st-order antiderivative-antialiased tanh. One nonlinearity per channel instance.
class ADAA_Tanh
{
public:
    void reset() noexcept { x1 = 0.0; }

    // f  = tanh(x);   F1(x) = log(cosh(x))
    static inline double f  (double x) noexcept { return std::tanh (x); }
    static inline double F1 (double x) noexcept
    {
        // numerically stable log(cosh): |x| - log(2) + log(1 + e^{-2|x|})
        const double ax = std::abs (x);
        return ax - 0.6931471805599453 + std::log1p (std::exp (-2.0 * ax));
    }

    inline double process (double x) noexcept
    {
        constexpr double eps = 1.0e-5;
        double y;
        const double dx = x - x1;
        if (std::abs (dx) < eps)
            y = f (0.5 * (x + x1));                    // midpoint fallback
        else
            y = (F1 (x) - F1 (x1)) / dx;               // averaged over the segment
        x1 = x;
        return y;
    }
private:
    double x1 { 0.0 };
};
```

For the **FET shaper**, swap in its `f`/`F1` (an asymmetric curve from the triode law) and add a `drive`/`bias` term; the ADAA machinery is identical. For symmetric/asymmetric blends and your Chebyshev harmonic control, keep `f` a closed-form curve so `F1` stays analytic.

### 7.4 TPT one-pole + feedback envelope detector

```cpp
struct TPTOnePole
{
    void prepare (double fs) noexcept { sampleRate = fs; setCutoff (1000.0); }
    void setCutoff (double fc) noexcept
    {
        const double g = std::tan (juce::MathConstants<double>::pi * fc / sampleRate);
        G = g / (1.0 + g);
    }
    inline double process (double x) noexcept
    {
        const double v = (x - s) * G;
        const double y = v + s;
        s = y + v;
        return y;
    }
    void reset() noexcept { s = 0.0; }
    double sampleRate { 48000.0 }, G { 0.0 }, s { 0.0 };
};

// Feedback (1176-style) gain computer with separate attack/release ballistics.
class FetGainReduction
{
public:
    void prepare (double fs) noexcept
    {
        sampleRate = fs;
        gAtt = coeff (attackSeconds);
        gRel = coeff (releaseSeconds);
        env = 0.0; lastGain = 1.0;
    }

    void setTimes (double attSec, double relSec) noexcept
    {
        attackSeconds = attSec; releaseSeconds = relSec;
        gAtt = coeff (attSec);  gRel = coeff (relSec);
    }
    void setLoopGain (double k_) noexcept { k = k_; }   // ratio = 1 + k

    // detectorInput = PREVIOUS output sample (feedback, breaks delay-free loop, 0 audio latency)
    inline double computeGain (double detectorInput) noexcept
    {
        const double rect = std::abs (detectorInput);          // full-wave
        const double g    = (rect > env) ? gAtt : gRel;        // attack vs release branch
        env += g * (rect - env);                               // one-pole; ripple intentionally retained
        // Map level over threshold to gain reduction (dB-domain feedback law, R = 1+k):
        const double overdB = std::max (0.0, todB (env) - thresholddB);
        const double grdB   = -k / (1.0 + k) * overdB;         // achieved reduction
        lastGain = std::pow (10.0, grdB / 20.0);
        return lastGain;
    }
    double lastGain { 1.0 };

private:
    static inline double todB (double x) noexcept { return 20.0 * std::log10 (std::max (1.0e-9, x)); }
    inline double coeff (double tau) const noexcept { return 1.0 - std::exp (-1.0 / (tau * sampleRate)); }

    double sampleRate { 48000.0 };
    double attackSeconds { 50e-6 }, releaseSeconds { 0.2 };
    double gAtt { 0.0 }, gRel { 0.0 }, env { 0.0 };
    double k { 7.0 }, thresholddB { -18.0 };
};
```

### 7.5 Oversampling wrapper (JUCE)

```cpp
// In the processor:
std::unique_ptr<juce::dsp::Oversampling<float>> os;

void prepareToPlay (double sr, int block) override
{
    constexpr int factorLog2 = 2;        // 4x
    os = std::make_unique<juce::dsp::Oversampling<float>> (
            getTotalNumOutputChannels(), factorLog2,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);  // min-phase, low latency
    os->initProcessing ((size_t) block);
    setLatencySamples ((int) os->getLatencyInSamples());   // ~0 for IIR; nonzero for FIR mode
    // prepare nonlinear stages at sr * 4 ...
}

void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
{
    juce::ScopedNoDenormals nd;
    juce::dsp::AudioBlock<float> block (buffer);
    auto up = os->processSamplesUp (block);
    // run the FET loop + ADAA saturators on `up` at the higher rate
    processNonlinearStages (up);
    os->processSamplesDown (block);
}
```

Use `filterHalfBandPolyphaseIIR` (≈ zero latency, min phase) for the default; expose a `filterHalfBandFIREquiripple` (linear phase, reports latency) "mastering" toggle. **Only oversample the nonlinear stages** — run the detector/ballistics at base rate if you want to save CPU (the control signal is slow and doesn't alias).

### 7.6 Performance: SIMD, denormals, structure

- **Denormals:** `ScopedNoDenormals` per block; also flush small states (envelope, filter `s`) to zero when below ~1e-15.
- **SIMD:** process stereo (or 4-wide) with `juce::dsp::SIMDRegister<float>` / `xsimd`; keep state in **structure-of-arrays**. Branchy ADAA fallbacks hurt SIMD — use `select()`/blend masks instead of `if` when vectorizing.
- **Block-rate vs sample-rate:** compute coefficients (`G`, ballistic coeffs, smoothed params) **once per block** (or per smoothing step), not per sample.
- **Warm-start Newton** (FET node) from the previous sample; cap iterations (2–3) for deterministic CPU.

### 7.7 Threading & state

- Message thread ↔ audio thread communicate only through **atomics / lock-free FIFO** (APVTS handles parameters). Never lock on audio.
- `reset()` and `prepareToPlay()` clear all filter/envelope state. Save/restore via `getStateInformation`/`setStateInformation` (APVTS XML).
- Handle sample-rate and block-size changes by re-preparing every DSP object.

---

## 8. Validating against hardware (how you know it's right)

1. **Static curve:** sweep a slow sine, plot output level vs input level per ratio → confirm slope `1/(1+k)` and the soft knee. Overlay a real-unit capture.
2. **Harmonic profile:** single tone at several levels/frequencies → FFT. Confirm **2nd-dominant** at moderate GR, rising **3rd** at low frequency / fast settings (the ripple mechanism, §2.2). Match the even/odd ratio vs the hardware.
3. **Step / impulse response:** gated tone burst → measure attack (20 µs–800 µs) and release (50 ms–1.1 s) envelopes; confirm the **reversed, log-ish knob mapping** and **program-dependent ratio creep**.
4. **Aliasing floor:** high-frequency tone near Nyquist at heavy drive → FFT → measure inharmonic alias products. Tune OS factor + ADAA order to your target (−90 to −120 dB).
5. **Null test:** if you have hardware captures, null model vs capture; residual reveals what's missing (usually transformer LF behavior or all-button bias dynamics).
6. **All-button:** verify the effective ratio lands ~12–20:1 with the **parallel bias network** shifting attack/release and the FET operating point (emergent dirt, not a bolt-on distortion stage).

---

## 9. One-page build recipe

1. **Audio path:** input iron (`Hin`, ADAA) → FET divider (1-D Newton node, signal-dependent `g`, optional ½-`V_DS` linearization tunable) → make-up gain → output Class-A + iron (`Hout`, ADAA).
2. **Control path:** rectify **previous output** (1-sample control delay, zero audio latency) → ratio bias (`k = R−1`) → nonlinear one-pole (attack/release, **ripple preserved**) → drive the FET gate.
3. **Antialiasing:** 2nd-order ADAA on the static shapers **+** 2×–4× min-phase IIR oversampling on the nonlinear block only.
4. **Latency:** 0 samples in tracking mode (no lookahead, IIR OS); linear-phase mastering mode reports its FIR delay.
5. **Engineering:** APVTS + `SmoothedValue`, `ScopedNoDenormals`, SoA + SIMD, no RT allocations/locks, coefficients per block, deterministic Newton.
6. **Validate:** static curve, harmonic profile, step response, alias floor, null test, all-button behavior.

---

*Math: standard JFET ohmic-region device physics, feedback control linearization, and AM-sideband analysis. ADAA from the continuous-time-convolution antialiasing framework (Parker, Esqueda, Bilbao). TPT/zero-delay filters per Zavalishin's VA filter design. Code targets JUCE (C++17) and is structured for real-time safety; treat constants (thresholds, K, V_P, transformer params) as values to fit against measured hardware.*


# Analog Audio Design — A Working Reference

*Amplifier topologies, alternate ways to get the same result, coloration craft, tone stacks and EQ (including inductor EQ), tubes, rectifiers, the full diode and transistor zoo, and high-end passive components. Written for boutique hardware + matching-plugin work.*

---

## Part A — Amplifier / gain-stage topologies

### A.1 The three building blocks (everything is made of these)

| Config (BJT / FET / tube) | Gain | Z in | Z out | Role / sound |
|---|---|---|---|---|
| **Common-emitter / source / cathode** | High V-gain, inverting | Med | Med-high | The workhorse voltage amp. Most of the "tone" lives here. |
| **Common-base / gate / grid** | High V-gain, non-inverting, no Miller | Low | High | Wideband; used in cascodes. Great HF. |
| **Common-collector / drain / cathode-follower** | ~Unity, buffer | High | Low | Impedance buffer / line driver. "Invisible" if done right. |

Stack these and you get every classic stage. The art is in the *loads* and *feedback* around them.

### A.2 Compound and differential structures

- **Cascode** (CE + CB stacked): kills Miller capacitance → huge bandwidth, high gain, low distortion. The secret weapon for clean, fast, high-impedance front ends. Tube cascode (two triodes) underlies many "modern hi-fi" preamps.
- **Differential pair / long-tailed pair (LTP):** two devices sharing a tail current source. Rejects common-mode noise/hum, cancels even harmonics (push-pull symmetry), and is the heart of every op-amp and balanced input. A *tail current source* (vs a tail resistor) raises CMRR dramatically.
- **Darlington** (two same-polarity BJTs): enormous β, but two Vbe drops, slower, noisier. The 1176 preamp uses one.
- **Sziklai / complementary-feedback pair** (NPN+PNP): Darlington-like gain with **one** Vbe drop, better linearity and thermal behavior. Quasi-complementary outputs use it. Often *sounds* cleaner than Darlington.
- **Current mirror / active load:** replace a plain resistor load with a current source → the stage's gain leaps (load = high dynamic impedance) and linearity improves. This single move separates "textbook" from "high-end."

### A.3 Tube-specific topologies (the clever ones)

- **SRPP** (shunt-regulated push-pull): a stacked-triode stage with an active upper device; high gain, low output impedance, some 2nd-harmonic character. Common in tube line stages and DI boxes.
- **Mu-follower / μ-follower:** like SRPP but optimized as a near-constant-current-loaded gain stage; very linear, low Zout.
- **White cathode follower:** an active, push-pull follower that can *source and sink* current — a strong line/headphone driver from tubes.
- **Cathodyne / concertina phase splitter:** one triode, equal anode/cathode outputs, 180° apart — cheap, balanced, slightly asymmetric drive. Classic in guitar amps (Fender).
- **LTP phase splitter:** the differential pair used to split phase for a push-pull output — tighter balance, more gain, the "Marshall" feel.
- **Aikido (Broskie):** a topology engineered so PSU noise cancels at the output and even harmonics partly cancel — clean tube line stage without heavy feedback.
- **Parafeed (parallel feed):** the output transformer is fed through a cap/choke so no DC flows in the iron → smaller, lower-distortion transformer, single-ended sweetness.
- **OTL (output-transformerless):** tube power amp with no output iron — very open, but needs many tubes and odd-impedance loads.

### A.4 Class, symmetry, feedback

- **Class A:** device conducts the whole cycle → no crossover distortion, dominant low-order harmonics, runs hot. The "premium" small-signal default.
- **Class AB / B:** efficient, but crossover distortion at the handoff — needs careful biasing/feedback. Push-pull cancels even harmonics, leaving odd.
- **Single-ended (SE):** asymmetric transfer → strong **2nd harmonic** → "warm, fat." Push-pull → even-harmonic cancellation → cleaner, more "powerful/odd."
- **Negative feedback (NFB):** global NFB lowers distortion, output impedance, and gain — but too much can sound "tight/sterile" and create TIM/slew issues. *Local* (per-stage) feedback or **zero global feedback** with inherently linear stages is the audiophile route. **NFB amount is itself a tone control.**
- **Exotic:** folded cascode, **Hawksford error correction**, current-feedback amps (very fast), nested differentiating feedback loops, fully balanced/differential signal paths (noise + even-harmonic cancellation end to end), transformer-coupled interstage.

---

## Part B — Many ways to get the same result (dynamics / gain control)

All compressors need a **variable-gain element** + a **detector**. The element defines the sound:

| Element | Control mechanism | Speed | Distortion signature | Famous in |
|---|---|---|---|---|
| **FET (VVR)** | Channel resistance vs Vgs | **µs** (fastest) | Even (V_DS²), gritty when pushed | 1176 |
| **Opto (LDR/Vactrol)** | Photocell resistance vs LED/EL brightness | Slow, ms, *memory/lag* | Low, smooth, frequency-dependent | LA-2A, LA-3A |
| **Vari-mu (remote-cutoff tube)** | Transconductance vs grid bias | Med | Soft, increasing-ratio, tube warmth | Fairchild 670, Manley |
| **Diode bridge** | Bridge resistance vs control current | Fast | Distinct, "aggressive" | Neve 2254 / 33609 |
| **VCA (Blackmer/log-antilog)** | Exponential gain vs control V | Fast, precise | Clean (or designed-in) | dbx 160, SSL bus, THAT 2180 |
| **OTA (transconductance amp)** | Bias current sets gm | Fast | Moderate | CA3080 / LM13700 designs |
| **PWM / DSP** | Digital | Arbitrary | Whatever you model | Modern |

**Design insight:** the *detector topology* (feedback vs feedforward) and the *ballistics* matter as much as the element. Feedback (1176/LA-2A) → soft knee, program-dependent, no lookahead needed (zero latency in DSP). Feedforward (VCA) → precise ratio, can do lookahead/brickwall. You can pair *any* element with *either* detector — that's a huge design space for "new" compressors.

---

## Part C — Coloration & tone (subtle yet powerful)

### C.1 The harmonic palette (what each one does to the ear)

- **2nd harmonic (octave):** consonant, adds "warmth, fullness, body." The single-ended/tube/JFET signature. Subtle amounts read as "richer, bigger."
- **3rd harmonic (octave+fifth):** adds "power, edge, thickness." Push-pull and symmetric clipping emphasize it. A little = punch; a lot = "crunch."
- **Higher odd (5th, 7th…):** "harsh, buzzy, fatiguing." Hard clipping. Use sparingly/intentionally.
- **Even vs odd ratio is the master coloration knob.** Asymmetry → even-rich (warm). Symmetry → odd-rich (aggressive).

### C.2 Mechanisms you can deploy

- **Transformer saturation:** B-H hysteresis → predominantly **3rd harmonic at LF/high level**, gentle HF roll, phase shift, and a "weight/glue" that's hard to fake. Core material sets the character (nickel = sweet/low-distortion, steel = more iron, more LF saturation).
- **Tube/FET soft clipping:** square-law transfer (triode, JFET) → 2nd-dominant, gradual onset, "musical" overdrive.
- **BJT clipping:** exponential transfer → sharper knee, more odd content.
- **Controlled crossover distortion:** under-bias a push-pull stage on purpose → adds high-order content at low level — the gritty edge in some "vintage" gear.
- **Bias manipulation / asymmetry:** offset the operating point to lean even or odd; switchable bias = switchable color.
- **Tilt / air bands:** a gentle high-shelf "air" (10–20 kHz) and a slow tilt across the spectrum read as "expensive/open" with almost no measurable EQ.
- **Parallel ("New York") processing:** blend a heavily processed copy under the dry — get density/harmonics without losing transients.
- **Harmonic exciters (Aphex-style):** generate and blend HF harmonics for "presence" without raw boost.
- **Tape/console emulation:** combine LF head bump, HF saturation, wow/flutter, transformer iron.

### C.3 "Subtle yet powerful" recipe

Low-order (2nd + a touch of 3rd) at **low level**, generated by a single-ended Class-A stage and/or input+output transformers, made **switchable/dial-able** (drive control + transformer in/out), with a slight tilt. That's the entire premise of a good "mastering color box" — and of your ColorDrive concept: a JFET Class-A gain stage for the even-harmonic core, a switchable transformer for the iron, and a blend/drive control to keep it subtle until you want it powerful.

---

## Part D — Tone stacks & EQ topologies

### D.1 Passive guitar-style stacks

- **FMV (Fender / Marshall / Vox) stack:** the classic Bass-Mid-Treble passive network between gain stages. It's **interactive** (controls fight each other), has **insertion loss** (~−20 dB, recovered by the next stage), and its midrange "scoop" is a defining part of the guitar-amp sound. Different cap/pot values = Fender vs Marshall vs Vox voicing.
- **Big Muff tone control:** a blend between a lowpass and highpass with a midrange notch — the famous "scooped" sweep.
- **Tilt control (Quad/Tonelux):** one knob tilts the spectrum (treble up + bass down, or vice-versa) around a pivot — extremely musical, minimal.

### D.2 Active EQ

- **Baxandall:** active bass/treble shelving in a feedback loop — smooth, wide, low-distortion, the hi-fi standard. **James network** is the passive Baxandall.
- **Shelving vs peaking (bell):** shelves for broad tonal balance, bells for surgical/musical boosts/cuts.
- **Q behavior:**
  - *Constant-Q:* bandwidth fixed regardless of boost/cut (graphic EQs, surgical work).
  - *Proportional-Q:* bandwidth widens at low boost, narrows at high boost (API-style — "musical," forgiving).
  - *Constant-bandwidth:* fixed Hz span.
- **Parametric / semi-parametric:** independent freq, gain, Q (full) or fixed-Q (semi).
- **Filter cores:** state-variable (independent LP/BP/HP, smooth), Wien-bridge, twin-T (deep notch), bridged-T (used in sidechains and the 1176 input).

---

## Part E — Inductor EQ (the real-iron sound)

### E.1 Why inductors at all

A real **LC resonant section** gives a peaking/notching response with a **naturally musical Q** and a phase/transient behavior that op-amp (gyrator) EQ approximates but doesn't perfectly match. Inductors also **saturate** slightly at high level → a subtle program-dependent sweetening. Downsides: they're big, expensive, and **pick up hum** (must be shielded/oriented).

### E.2 The classics

- **Pultec EQP-1A:** *passive* LC EQ followed by a tube make-up amplifier. Its legend is the **low-frequency boost + attenuation trick**: boosting and cutting the same low band (slightly different curves) produces a resonant bump above plus a dip just below → "bigger low end that's somehow tighter." The broad, gentle, overlapping curves + the tube/iron make-up stage are why a Pultec "sounds good doing nothing."
- **API 550A/550B/560:** **inductor-based, proportional-Q**, with discrete **2520 op-amp** make-up gain and input/output transformers. The "punch and forward midrange" comes from the proportional-Q curves + the discrete/iron path.
- **Neve 1073/1084:** **inductor + transformer** EQ (Marinair/St Ives/Carnhill iron). Fixed musical frequencies, broad curves, the Class-A discrete amps and transformers add the "Neve weight."

### E.3 Gyrators (the inductor substitute)

A **gyrator** (op-amp + capacitor + resistors) simulates an inductor — no hum pickup, tiny, cheap, tunable. Used in SSL and countless console EQs. It can match the *frequency* response of an LC section closely, but lacks the real inductor's saturation and has the op-amp's own character. For DSP modeling, both are just biquads — but if you want the *inductor* sound, model the **core saturation and the LC phase** explicitly, not just the magnitude curve.

---

## Part F — Tubes / valves

### F.1 Small-signal triodes

| Tube | μ (gain) | Character / use |
|---|---|---|
| **12AX7 / ECC83** | ~100 (high) | The default high-gain preamp triode. Lots of gain, higher distortion. |
| **5751** | ~70 | Lower-gain 12AX7 sub — smoother, more headroom. |
| **12AT7 / ECC81** | ~60 | Phase splitters, reverb drivers, brighter. |
| **12AU7 / ECC82** | ~17 (low) | Very linear, low gain — line stages, cathode followers. |
| **6SN7 / 6SL7** | ~20 / ~70 | Octal "hi-fi" triodes — big, open, dimensional. |
| **ECC88 / 6DJ8 / 6922 / E88CC** | high gm, low rp | Cascodes, SRPP, followers — fast, low Zout, "modern." |
| **417A / 5842, 437A, 6C45** | very high gm | Wideband/low-noise front ends; from instrumentation/RF lineage. |

### F.2 Pentodes & power tubes

- **EF86** — low-noise pentode preamp (vintage mics, Vox).
- **6V6 / 6L6 / 5881 / KT66** — classic power, from sweet (6V6) to firm (KT66).
- **EL84 / 6BQ5** — chimey, early breakup (Vox).
- **EL34** — the "British" power tube (Marshall), rich mids.
- **KT88 / KT120 / KT150** — high-power, tight, hi-fi.
- **DHT (directly-heated triodes): 2A3, 300B, 45** — single-ended hi-fi royalty: gorgeous 2nd-harmonic, low power, demands great output iron.
- **Transmitter triodes: 845, 211, GM70, 811A** — high-voltage SE behemoths with a distinctive midrange.

### F.3 Vari-mu / remote-cutoff (for compression)

**6386, 5703, 6BA6, 6SK7** and similar **remote-cutoff** tubes change transconductance smoothly with grid bias — the gain element in vari-mu compressors (Fairchild 670 uses 6386s). The "ratio increases with level" softness is intrinsic to the remote-cutoff curve.

### F.4 Why tubes sound like that

High plate impedance, **square-law-ish transfer → 2nd-harmonic-dominant**, gentle clipping (grid conduction softens peaks), and the **output transformer** as part of the tone. Microphonics and the slow PSU (tube rectifier sag) add "feel." Sub-miniature/pencil tubes (**5703, 6111, 6112**) let you build compact boutique tube gear.

---

## Part G — Rectifiers & the power supply as a tone element

### G.1 Tube rectifiers (sag = feel)

| Tube | Sag / behavior |
|---|---|
| **5Y3** | High sag — soft, spongy, "vintage bounce." |
| **5U4G** | Medium-high sag — classic American. |
| **GZ34 / 5AR4** | Low sag — tight, efficient, modern-tube. |
| **EZ81 / EZ80** | Small-amp rectifiers. |

**Sag** = supply voltage dipping under transient current draw → the amp's headroom momentarily compresses → that elastic "give" players love. Tube rectifiers also **turn on slowly**, protecting the other tubes and adding a soft power-up.

### G.2 Solid-state & hybrid

- **Silicon bridge (1N400x):** stiff, efficient, no sag — tight and loud.
- **Schottky / fast-soft-recovery (MUR, FRED):** lower switching noise/"hash" → quieter supply, better for hi-fi. Add **snubbers** across diodes to tame ringing.
- **Hybrid:** SS rectify + a series resistor or CCS to *emulate* sag, or a tube-rectifier "dropper" for the look/feel without the fragility.

### G.3 The PSU is part of the signal path

Filter capacitance, **choke-input vs cap-input** (choke = smoother, lower ripple, slight sag character), **regulation** (stiff = clean/controlled; unregulated = "breathes"), and **star grounding** all shape noise floor and "tightness." For high sound quality, a quiet, well-regulated supply with local decoupling at each stage matters as much as the gain devices. For *vintage feel*, deliberately softer supplies help.

---

## Part H — The diode zoo

| Type | Vf / trait | Audio use |
|---|---|---|
| **Silicon signal (1N4148/1N914)** | ~0.6–0.7 V, sharp knee | Standard clipping, the "tube screamer" symmetric pair, logic/protection |
| **Germanium (1N34A, OA90, OA91)** | ~0.2–0.3 V, **soft knee**, leaky | Vintage soft clipping, smoother fuzz, warm overdrive |
| **Schottky (1N5817, BAT41, BAT85)** | ~0.15–0.3 V, fast, low cap | Soft/low-threshold clip, fast PSU rectification, low switching noise |
| **LED** | ~1.6–2 V (color-dependent), distinct knee | **Higher-headroom clipping** (more open, dynamic), bias references, indicators |
| **Zener** | Reverse breakdown set by part | Voltage clamping/clipping, regulation, references |
| **Rectifier (1N400x) / soft-recovery (MUR, FRED)** | Power | PSU; soft-recovery for low hash |
| **Varactor / varicap** | Voltage-variable capacitance | Tuning, vibrato/chorus modulation |
| **PIN** | RF resistance vs current | RF/attenuator switching |
| **Tunnel** | Negative resistance | Exotic oscillators |

**Clipping topologies:** symmetric pair (odd harmonics, "harder"), **asymmetric** (one diode one side, or Ge one way + Si the other → even harmonics, "warmer"), **stacked series** diodes (raise threshold/headroom), **mixed types** (Ge+Si+LED) for graded knees, **diode-to-ground** (hard) vs **diodes in the op-amp feedback loop** (softer, gain-dependent — the smoother overdrive). A **diode ring/bridge** is also a *gain-control* element (Neve compressors) and a *modulator*.

---

## Part I — The transistor zoo

### I.1 BJTs (bipolar)

- **Small-signal jewels:** 2N3904/2N3906, **BC550C/BC560C** (low noise), 2N5087/2N5089, **2SC2240/2SA970** and **2SC1815/2SA1015** (Toshiba audio favorites), ZTX series. Matched pairs: **THAT300/THAT320** arrays for differential/log work.
- **Germanium (vintage):** **AC128, OC44, OC75, 2N404, AC176** — leaky, temperature-touchy, the soul of Fuzz Face-era fuzz. Asymmetric, soft, "broken" in a good way.
- **Power:** 2N3055/MJ2955 (classic), **2SC5200/2SA1943** (Toshiba hi-fi pair), Sanken/ThermalTrak (built-in thermal sensing).
- **Transfer law:** **exponential** (Ic ∝ e^(Vbe)) → sharper distortion knee, more odd harmonics than tubes/JFETs.

### I.2 JFETs (depletion-mode, the "tube-like" transistor)

- **Low-noise audio jewels:** the legendary **Toshiba 2SK170 / 2SJ74** (complementary) and **2SK369, 2SK246/2SJ103** are **discontinued**. Current production from **Linear Systems**: **LSK170 / LSJ74** (singles, ~1 nV/√Hz), **LSK389** (dual monolithic N, ~1 nV/√Hz, best matching/thermal tracking), **LSK489 / LSJ689** (duals, slightly higher noise but lower capacitance), **LSK189**. Also **BF862** (low-noise, itself now EOL but findable) and SMD **2SK209/2SK880/2SK932**.
  - Note: a 2SK170/2SJ74 pair reaches ~15 Ω output impedance; an LSK489 combo lands nearer ~330 Ω — pick by application (mic pre vs line buffer).
  - **Beware counterfeits** on the open market; buy matched from reputable sources.
- **VVR / fuzz / utility:** **J201, 2N5457, 2N5458, MMBF170** — the gain-reduction and soft-clip workhorses (J201 famously "amp-like" in pedals; 2N5457 is the common 1176-clone GR FET).
- **Transfer law:** **square-law** (Id ∝ (Vgs−Vp)²) — *same family as a triode* → **2nd-harmonic-dominant, "warm"**, gentle clipping. This is why JFET stages are prized for "tube-like" preamps and your Class-A color stage.

### I.3 MOSFETs

- **Lateral (audio power):** **Hitachi/Exicon 2SK1058/2SJ162, ALFET, Renesas** — thermally stable, "tube-like" linearity, used in high-end Class-A/AB power amps (no thermal runaway, soft clipping).
- **Vertical (switching/Class-D):** IRF series — efficient, for class-D output stages.
- **Small-signal:** BS170, 2N7000 — switching, utility, occasional gain.
- **Sound:** lateral MOSFETs are praised for a smooth, slightly tube-like top; vertical/switching types are about efficiency, not signal-path warmth.

### I.4 Square-law vs exponential (the why)

JFETs and triodes share a **square-law** transfer → strong, smooth **2nd harmonic** → "warm/musical." BJTs are **exponential** → sharper onset, more **odd** content → "edgier." Choosing the device *is* choosing the harmonic character before you've drawn a single resistor.

---

## Part J — High-end passive components

### J.1 Capacitors (dielectric hierarchy for signal path)

| Dielectric | Tier / use |
|---|---|
| **PTFE / Teflon (V-Cap, Jupiter)** | Top-tier signal coupling — fast, low-loss, expensive, large. |
| **Polystyrene** | Excellent (lowest dielectric absorption), fragile, small values — filters/EQ. |
| **Polypropylene (Wima, Mundorf, ClarityCap)** | The high-end workhorse — low loss, neutral; signal coupling and EQ. |
| **Silver mica** | Small values, stable, HF/RF, oscillator and treble networks. |
| **C0G / NP0 ceramic** | Excellent small-value (filters, compensation) — stable, low distortion. |
| **Polyester / Mylar** | Cheap, lossier — tone caps, non-critical coupling. |
| **X7R / Y5V ceramic** | **Avoid in signal path** — microphonic, voltage-dependent (distortion). PSU bypass only. |
| **Electrolytic (Nichicon Muse/FG, Elna Silmic II, Panasonic FC/FM; Black Gate = discontinued legend)** | Coupling (large values) and PSU. Bypass with film for HF. |

Roles: **coupling** (in signal path — dielectric matters most), **bypass/decoupling** (PSU — low ESR matters), **PSU reservoir** (capacitance + ripple). Dielectric absorption and microphonics are the audible failure modes in the signal path.

### J.2 Resistors

| Type | Noise / trait | Use |
|---|---|---|
| **Metal film** | Low noise, neutral, tight tolerance | The default for everything signal-path. |
| **Thin-film / bulk-foil (Vishay Z-foil "naked," Texas Components)** | Ultra-low noise, ultra-low tempco | First gain stage, reference dividers — the "ultimate." |
| **Carbon film** | More noise, cheaper | Non-critical. |
| **Carbon composition** | **Noisy, voltage-coefficient (adds distortion)** | Deliberate "vintage tone" in guitar amps; avoid in hi-fi signal path. |
| **Wirewound** | Low noise but **inductive** | Power/PSU, not HF signal. |

Where it matters: resistor **current (excess) noise** and Johnson noise dominate at the **first gain stage** and in **high-impedance** nodes. Spend your bulk-foil budget there.

### J.3 Inductors & chokes

Air-core (no saturation/distortion, but hum pickup and large), iron/ferrite (compact, saturate → coloration), nickel (low distortion). PSU **chokes** smooth ripple and add sag character; **EQ inductors** (Part E) bring the resonant "iron" EQ sound.

### J.4 Transformers (the iron)

- **Brands:** Jensen (clean, hi-fi), Lundahl (nickel/amorphous, refined), Cinemag, Sowter, **Carnhill/St Ives & Marinair** (the Neve sound), UTC/Triad (vintage), Edcor (budget), AMI/Tab-Funkenwerke (German vintage).
- **Core material:** **nickel/permalloy/mu-metal** = low distortion, extended/sweet HF, less LF saturation (mic inputs, hi-fi); **M6 grain-oriented steel** = more "iron," richer LF saturation, the "weight" (line/output coloration).
- **Construction:** interleaving and Faraday shields set bandwidth and hum rejection.
- **Roles:** input (balancing, impedance, slight color), interstage (gain/phase split, coupling), output (impedance matching + the bulk of the saturation character — and, in the 1176, feedback windings).
- **Why they color:** core hysteresis (LF 3rd harmonic), bandwidth limiting, LF phase shift, and saturation "give" at high level — the hardest thing to fake and the easiest way to add "expensive."

### J.5 Potentiometers & attenuators

**Alps RK27 "Blue Velvet"** (conductive plastic, smooth, low-noise) for quality volume/EQ; **log/audio taper** for level controls; **stepped attenuators** (Goldpoint, DACT, Khozmo, TKD) for precise **channel matching** and longevity (no scratch/wear). Conductive plastic beats carbon for noise and life.

---
a

### K.2 The "iron and glass + dial-able color" build (your lane)

A practical high-end color/mastering stage:

1. **Input:** transformer (nickel for clean, steel for character) → optional **drive** to push the iron.
2. **Gain core:** **Class-A JFET stage** (LSK170/LSK389-class device) — square-law → 2nd-harmonic warmth; bias-set asymmetry for even/odd lean; minimal local feedback to keep it open.
3. **Color controls:** drive (how hard you hit iron/JFET), **transformer in/out**, bias/asymmetry switch, and a subtle **tilt/air** shelf.
4. **Output:** transformer-balanced line driver (the second helping of iron) + a clean buffer.
5. **PSU:** quiet, well-decoupled, star-grounded; choose stiff (clean) vs slightly soft (feel) deliberately.
6. **Parts where it counts:** bulk-foil resistors and a low-noise JFET at the first stage; polypropylene/PTFE coupling; good iron.

This is exactly the **ColorDrive** premise: a JFET Class-A core for the harmonic body, a switchable transformer for the iron, and a drive/blend so it's *subtle by default, powerful on demand*.

### K.3 Modeling each in DSP (for the matching plugin)

- **Square-law JFET/tube stage** → asymmetric waveshaper (even-dominant), with **ADAA** for antialiasing (you've got this).
- **Transformer** → **Jiles-Atherton** hysteresis (you've got this) for the LF 3rd + saturation; add the LF phase and bandwidth limit.
- **Inductor EQ** → biquads for the magnitude **plus** explicit core-saturation nonlinearity if you want the real-iron behavior, not just the curve.
- **Diode clipping** → static shaper matched to the diode's knee (Ge soft / Si sharp / LED high-headroom), asymmetric for warmth, ADAA'd.
- **PSU sag** → an envelope-following supply-rail model that compresses headroom under load (the "breathe").
- **Tilt / air** → gentle shelves; the cheapest "expensive" you can add.

---

*Engineering content synthesized from standard analog/audio design practice. Current-production low-noise JFET specifics (Toshiba 2SK170/2SJ74 discontinued; Linear Systems LSK170/LSJ74/LSK389/LSK489 replacements, noise figures, output-impedance comparisons) verified against Linear Systems and DIY-audio community sources. Treat all device values and part choices as starting points to fit/measure for your specific build.*



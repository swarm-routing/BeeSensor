# BeeSensor: fidelity sheet

What the paper specifies, what this implementation does, and where they differ.
Every row cites its source (`[KEY] §x`, `Table N`, `Fig. N`, `eq. N`), or says
`repo choice: <reason>`, or `unknown`. Checked by
`swarm-routing/.github` `tools/fidelity/fidelity.py check`.

**State: started, not read.** Both BeeSensor papers are closed-access
(OpenAlex lists [SUF12] and [SF07] as closed, no repository copy, checked
2026-11-24), and neither has been read for this sheet yet: getting the texts is
a maintainer step (#25). Until then **every row is `unknown`**. The rows below
name what the sheet has to settle, taken from the plan (epic #3,
swarm-routing/.github#672) and the org's protocol checklist, not from the
papers; none of them is a claim about what BeeSensor does. No row may be filled
from memory, from a secondary description, or from BeeAdHoc's sheet (golden
rules 10 and 12).

**Implementation:** none. No protocol code is written until this sheet has
been reviewed (golden rule 10); the `core/` epic tracks the code.

**Status** says how each row stands against the source, as in
swarm-routing/BeeAdHoc's sheet:

- `as written; settled (<locator>)`: the source gives it, and `core/` follows it;
- `deviation: …` / `not implemented: …`: `core/` departs from it, with the reason;
- `repo choice (maintainer decision <date>, <issue>): …`: the source leaves it
  open, and the maintainer chose, pre-registered on that issue;
- `unknown: …`: not settled yet, with what will settle it.

## Sources

<!-- fidelity:sources:begin -->
| Key | Citation | Link |
|---|---|---|
| SF07 | Saleem & Farooq, *BeeSensor: A Bee-Inspired Power Aware Routing Protocol for Wireless Sensor Networks*, Applications of Evolutinary Computing, pp. 81–90 | <https://doi.org/10.1007/978-3-540-71805-5_9> |
| SUF12 | Saleem, Ullah & Farooq, *BeeSensor: An energy-efficient and scalable routing protocol for wireless sensor networks*, Information Sciences 200, 2012, pp. 38–56 | <https://doi.org/10.1016/j.ins.2012.02.024> |
| W05 | Wedde et al., *BeeAdHoc: an energy efficient routing algorithm for mobile ad hoc networks inspired by bee behavior*, Proceedings of the 7th annual conference on Genetic and evolutionary computation, 2005, pp. 153–160 | <https://doi.org/10.1145/1068009.1068034> |
<!-- fidelity:sources:end -->

The entries live in [`references.bib`](references.bib); the table is
generated from it (`fidelity.py bib sources`). [SUF12] is the primary version:
the journal paper with the full protocol and its WSN evaluation, the version
this repository implements. [SF07] is the first, shorter workshop paper; it is
read for the differences from [SUF12] (the *Which version* section below).
[W05] is BeeAdHoc, the same group's MANET protocol, cited for the lineage only;
no row cites it as a BeeSensor source.

## Which version of BeeSensor

[SUF12] is primary (#25). What [SF07] specifies differently, if anything, is
`unknown` until both are read; each difference then becomes a row of
*Differences between [SF07] and [SUF12]* below, and `core/` follows [SUF12]
unless a row says otherwise.

## Relation to BeeAdHoc

BeeSensor carries the bee metaphor of BeeHive and BeeAdHoc ([W05],
swarm-routing/BeeAdHoc) from any-to-any MANET routing to many-to-one
sensor-to-sink routing with the nodes' remaining energy in the route choice.
Which BeeAdHoc mechanisms it keeps, changes or drops is `unknown` until
[SUF12] is read; once it is, each reuse is a row here citing [SUF12], and
BeeAdHoc's own sheet is never the source of a BeeSensor row. Where
swarm-routing/BeeAdHoc's `core/` already implements a mechanism BeeSensor
shares, reusing that code keeps its licence and provenance (golden rule 12).

## Mechanisms

| Mechanism | Source | Implementation | Status |
|---|---|---|---|
| Route discovery towards the sink: which agent explores, who starts it, when | unknown | — | unknown: not read (#25) |
| Data delivery from a sensor to the sink along the discovered paths | unknown | — | unknown: not read (#25) |
| Path scoring: how remaining energy and path length (or other metrics) weigh a route; the epic's "scout admission floor" and "sink path scoring" | unknown | — | unknown: not read (#25); the energy reaches `core/` through swarm-core's `IEnergyProvider` |
| Route maintenance: refresh, expiry and the response to a broken link or a depleted node | unknown | — | unknown: not read (#25) |
| Per-node state: what a node stores, and its bounds (golden rule 7) | unknown | — | unknown: not read (#25) |
| Control packets and their fields (the wire header, golden rule 3) | unknown | — | unknown: not read (#25) |
| What BeeSensor keeps, changes or drops from BeeAdHoc | unknown | — | unknown: not read (#25) |

## Parameters

| Parameter | Value | Source | Notes |
|---|---|---|---|
| Discovery agents' launch rate or interval | — | unknown | unknown: not read (#25) |
| Agents' lifetime / hop limit | — | unknown | unknown: not read (#25) |
| Energy weight(s) in the path score | — | unknown | unknown: not read (#25) |
| Route expiry / refresh timers | — | unknown | unknown: not read (#25) |
| Paths kept per destination (the sink) | — | unknown | unknown: not read (#25) |

## Anchor candidate

| Trend | Scenario | Metric | Comparators | Source |
|---|---|---|---|---|
| The headline trend of [SUF12]'s WSN evaluation, to be chosen when it is read | the many-to-one WSN convergecast family (swarm-routing/sim-tools#55), set up as the paper's | unknown | unknown | unknown |

The anchor (org golden rule 6) is chosen from [SUF12]'s own evaluation once it
is read, and pre-registered on its issue before any run; a miss ships as a
written infeasibility verdict, as ARA, BeeAdHoc and HOPNET did.

## Differences between [SF07] and [SUF12]

None recorded: neither paper has been read (#25).

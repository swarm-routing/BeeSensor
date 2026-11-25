# BeeSensor: fidelity sheet

What the sources specify, what this implementation does, and where they differ.
Every row cites its source (`[KEY] §x`, `p. N`, `Table N`, `Fig. N`), or says
`repo choice: <reason>`, or `unknown`. Checked by
`swarm-routing/.github` `tools/fidelity/fidelity.py check`.

**Version: v0, from public secondary sources.** Both BeeSensor papers,
[SUF12] (the version this repository implements) and [SF07], are
closed-access (OpenAlex, Semantic Scholar and Unpaywall list no open copy,
checked 2026-11-24), and neither has been read for this sheet. Under the
maintainer's decision of 2026-11-24
(<https://github.com/swarm-routing/.github/issues/881#issuecomment-7147077907>;
org ROADMAP "Public-source v0 protocols": a protocol without an open paper
is built first from public sources), the rows below are filled from open
descriptions of BeeSensor. When the paper is in hand, every row is
re-checked against it and each deviation recorded. Each row says where it comes
from:

- `paper`: read in [SUF12] or [SF07]. **No row is `paper` yet.**
- `secondary: [KEY]`: an open source with a bibliographic record in
  [`references.bib`](references.bib), at the cited page.
- `repo choice: …`: the open sources leave it open, or it comes from an open
  description that has no bibliographic record and so cannot be a source
  here (its URL is named in the reason). The value is this repository's,
  for the review to accept or change.
- `unknown`: not settled by any open source; it stays open until the paper is
  read.

Nothing here is from memory, and nothing is taken from BeeAdHoc's sheet
(AGENTS.md rules 10 and 12). The two open descriptions without a record are:

- **IJARCCE13** Sridhar, Siddappa & Bhanu Prakash, *Power Aware Routing Protocol
  for MANET's using Swarm Intelligence*, IJARCCE 2(8), 2013, pp. 2960–2965,
  <https://www.ijarcce.com/upload/2013/august/6-o-Sridhar%20H.S-power%20aware%20routing.pdf>.
  It presents the protocol under the name "SensorBee", but its text is a
  description of BeeSensor (it names BeeSensor in its figures and
  conclusion). It is the most detailed open description and has the least
  authority. Every row that follows it is a `repo choice`.
- **SJET20** Singh, Garg & Parvez, *Challenges on ACO Based and BCO Based
  Routing Protocols in MANETS*, Sch J Eng Tech 8(11), 2020, pp. 222–230,
  <https://saspublishers.com/article/13565/download/>. Its §4.2 describes
  BeeSensor's agents with the same wording as IJARCCE13, citing [SF07].

These two are labels in this prose only, not bibliography keys: the tooling
(`fidelity.py bib entry`) finds no record for either (no registered DOI, no
citation metadata), and an entry is never typed by hand.

**Implementation:** `core/` v0 (#40), built from this sheet. Each row names
what `core/` does; the constants are in `core/beesensor-config.h`, each
citing its Parameters row.

## Sources

<!-- fidelity:sources:begin -->
| Key | Citation | Link |
|---|---|---|
| RF15 | Marjan Kuchaki RAFSANJANI, *A Survey on Algorithms Based on Bee Swarms for Ad Hoc Networks*, Technical report, Walailak University, 2015 | <https://doi.org/10.14456/wjst.2015.2> |
| SF07 | Saleem & Farooq, *BeeSensor: A Bee-Inspired Power Aware Routing Protocol for Wireless Sensor Networks*, Applications of Evolutinary Computing, pp. 81–90 | <https://doi.org/10.1007/978-3-540-71805-5_9> |
| SUF12 | Saleem, Ullah & Farooq, *BeeSensor: An energy-efficient and scalable routing protocol for wireless sensor networks*, Information Sciences 200, 2012, pp. 38–56 | <https://doi.org/10.1016/j.ins.2012.02.024> |
| W05 | Wedde et al., *BeeAdHoc: an energy efficient routing algorithm for mobile ad hoc networks inspired by bee behavior*, Proceedings of the 7th annual conference on Genetic and evolutionary computation, 2005, pp. 153–160 | <https://doi.org/10.1145/1068009.1068034> |
| ZSAC13 | Zungeru et al., *Energy Efficiency Performance Improvements for Ant-Based Routing Algorithm in Wireless Sensor Networks*, Journal of Sensors 2013, 2013, pp. 1–17 | <https://doi.org/10.1155/2013/759654> |
<!-- fidelity:sources:end -->

The entries live in [`references.bib`](references.bib); the table is
generated from it (`fidelity.py bib sources`). [SUF12] is the primary version
once it is read. [RF15] and [ZSAC13] are the open secondary sources the v0
rows cite. [ZSAC13] also ran BeeSensor as a comparator; see *Anchor
candidate*. [W05] is BeeAdHoc, cited for the lineage only.

## Which version of BeeSensor

[SUF12] is primary (#25). The open sources describe BeeSensor without
separating the 2007 and 2012 versions: [RF15] p. 24 and [ZSAC13] p. 3 (its
reference 16, p. 17) cite [SUF12], while SJET20 §4.2 cites [SF07]. The sources also disagree on the
agents. [RF15] p. 24 and SJET20 §4.2 list four: packers, scouts, foragers,
swarms. [ZSAC13] p. 3 lists three and omits swarms. v0 follows the four.
Reading the papers settles which version each mechanism belongs to (v0.3.0,
`docs/roadmap.md`).

## Relation to BeeAdHoc

SJET20 §4.2 and IJARCCE13 p. 2963 say foragers are the main workers "as in
BeeAdHoc" ([W05], swarm-routing/BeeAdHoc). IJARCCE13 p. 2964 says that, like
BeeAdHoc, BeeSensor uses no HELLO or route-error messages. The differences
the open sources state:

- forward scouts carry no source route, so their size does not grow with the
  path (SJET20 §4.2, IJARCCE13 p. 2962);
- paths go towards a sink (many-to-one) rather than any-to-any ([RF15] p. 24);
- paths are node-disjoint, chosen by a reward that uses the least residual
  energy on the path (IJARCCE13 pp. 2962–2963).

No BeeAdHoc code or value is reused without a row here citing a BeeSensor
source (rule 12).

## Mechanisms

| Mechanism | Source | Implementation | Status |
|---|---|---|---|
| Four agent types: packers, scouts, foragers, swarms | [RF15] p. 24 | `core/` v0 (`beesensor-router.cc`) | secondary: [RF15]; [ZSAC13] p. 3 lists three (no swarms); SJET20 §4.2 agrees with four |
| Packers receive and store data packets from the transport layer and hand each to a forager (a route); no forager means no route to a sink | [RF15] p. 24 | `core/` v0 (`beesensor-router.cc`) | secondary: [RF15]; [ZSAC13] p. 3 ("packers locate appropriate foragers for the data packets at the source node") |
| Route discovery is reactive, on demand: a source with an event and no route launches a forward scout carrying the event in its payload and broadcasts it | [ZSAC13] p. 3 | `core/` v0 (`beesensor-router.cc`) | secondary: [ZSAC13] |
| Scouts explore new routes to a sink node, as forward and backward scouts | [RF15] p. 24; [ZSAC13] p. 3 | `core/` v0 (`beesensor-router.cc`) | secondary: [RF15], [ZSAC13] |
| A forward scout carries no source route (fixed size); its header holds scout ID, source ID, minimum remaining energy (initially ∞) and hop count (initially 0) | repo choice: follows IJARCCE13 pp. 2962–2963 (no source route, p. 2962; the header fields, p. 2963) and SJET20 §4.2; no recorded source | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Forward-scout rebroadcast: an intermediate node updates its scout cache and increments the hop count; within HL hops of the source it rebroadcasts unconditionally, beyond HL with probability Pi; it lowers the energy field to its own level if lower | [ZSAC13] p. 3 | `core/` v0 (`beesensor-router.cc`) | secondary: [ZSAC13] for the scout cache, the hop count and the unconditional rebroadcast within a radius of the source; repo choice: the probabilistic rebroadcast beyond HL and the energy minimum follow IJARCCE13 p. 2963 |
| The sink delivers the event, adds a forwarding entry (path ID, next hop = sink, previous hop = sender), empties the payload, resets the energy field to ∞, writes a unique path ID and turns the scout into a backward scout sent to the previous hop | repo choice: follows IJARCCE13 p. 2963; [ZSAC13] p. 3 says only that small forwarding tables are built as the scout returns | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Backward scouting: each node on the way back adds a forwarding entry from its scout cache, accepts only the first backward scout of a generation (paths node-disjoint), lowers the energy field, and forwards to the previous hop | [ZSAC13] p. 3 | `core/` v0 (`beesensor-router.cc`) | secondary: [ZSAC13] for the forwarding tables built during the scout's return; repo choice: disjointness and the per-generation rule follow IJARCCE13 p. 2963 |
| Node tables: the source keeps a routing table and a probability distribution table; sink and intermediate nodes keep forwarding tables (path ID, next hop, previous hop) | repo choice: follows IJARCCE13 p. 2963 | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Foragers carry data point-to-point along a path, indexed by path ID (PID) in the forwarding tables; foragers of one path are grouped | [RF15] p. 24; [ZSAC13] p. 3 | `core/` v0 (`beesensor-router.cc`) | secondary: [RF15], [ZSAC13] for point-to-point foragers; repo choice: the PID indexing and the grouping follow SJET20 §4.2 and IJARCCE13 p. 2963 (no bibliographic record) |
| Path choice at the source is stochastic, by a reward of path length (hops) and the least residual energy on the path | repo choice: follows IJARCCE13 pp. 2962–2963 and SJET20 §4.2 ("selected stochastically at the source node"); the reward's form is a repo choice (Parameters) | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Events generated during route discovery wait in a small event cache at the source | repo choice: follows IJARCCE13 p. 2963 | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Swarms return foragers to the source when they are not piggy-backed on link-layer acknowledgements; foragers wait at the sink, then travel back as one swarm on the reverse links | [RF15] p. 24 | `core/` v0: the sink holds a path's foragers for the sink wait, then sends one `Swarm` to the previous hop; no link-layer piggy-back (the adapter has no hook for it), so every return is a swarm | secondary: [RF15]; detail follows SJET20 §4.2; deviation: no piggy-back |
| A swarm does not advertise a path whose minimum remaining energy is below pL while better paths exist | repo choice: follows IJARCCE13 p. 2962 | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Loop freedom: a backward scout that reaches a node twice is dropped and its entry flushed | repo choice: follows IJARCCE13 p. 2963 | `core/` v0 (`beesensor-router.cc`) | repo choice |
| Path maintenance without HELLO or RERR: a path stays valid while it has foragers; with none left it is removed after the wait FORG; forward scouting restarts only when every path is broken and events wait; foragers and forwarding entries expire | repo choice: follows IJARCCE13 p. 2964; [RF15] p. 24 names path maintenance as a phase | `core/` v0: each forager sent counts as outstanding until a swarm returns it. A path with foragers out and no swarm back for FORG is removed; FORG counts from the first send while none was out, then from each swarm's arrival, so a path that keeps returning foragers survives one lost on a lossy link (repo choice). A path idle (no forager sent or returned) for the forager TTL is removed too. Forwarding entries expire after their lifetime unless a forager or swarm refreshes them, taking the sink's waiting foragers and their swarm timer with them. A new forward scout leaves only when no path is left: on the next event once the last scout went unanswered for the scout timeout, or from a timer while events wait | secondary: [RF15] (the phase); repo choice: the rules |
| Control packets and their fields as a versioned wire header (golden rule 3) | repo choice: the layout is this repository's (one version byte, little-endian), carrying the fields the rows above name | `core/` v0 (`beesensor-wire.h`): kind byte after the version; energy in 1/10000 of a full battery, 0xFFFF the infinite start value; roundtrip, fail-closed and seeded fuzz tests | repo choice |

## Parameters

| Parameter | Value | Source | Notes |
|---|---|---|---|
| HL, the hop radius of unconditional forward-scout rebroadcast | 2 | repo choice: IJARCCE13 p. 2964 ("nodes … located beyond 2 hops perform stochastic broadcasting"); its Fig. 2 shows HL = 3 as an example | repo choice; IJARCCE13 p. 2963 proposes HL as a function of the estimated source–sink hops (not in v0) |
| Pi, rebroadcast probability beyond HL | 0.5 | repo choice: no open source gives a value | repo choice; to be measured by A/B |
| Initial minimum-energy field of a scout | ∞ (residual fraction > 1) | repo choice: IJARCCE13 p. 2963 | repo choice; energy is swarm-core `IEnergy`'s residual fraction in [0, 1], NaN when unknown |
| Path reward | r = E_min / hops | repo choice: IJARCCE13 p. 2962 says only "a function of the path length (hops) and the least energy level of a node on the path" | repo choice; E_min is the least known reading on the path: an unknown (NaN, no energy model) reading leaves the running minimum unchanged, and a minimum still at its ∞ start value when the path is scored counts as 1. So an unknown reading acts as 1 (a full battery), and a path with no known reading scores 1/hops, never ∞ |
| Path choice probability | p_k = r_k / Σ r | repo choice: SJET20 §4.2 says "selected stochastically"; the distribution is ours | repo choice |
| pL, energy floor below which a swarm does not advertise a path | 0.1 | repo choice: IJARCCE13 p. 2962 names pL, no value | repo choice |
| FORG, wait before removing a path with no foragers | 1 s | repo choice: IJARCCE13 p. 2964 names it, no value | repo choice |
| Forager wait at the sink before a swarm leaves | 0.5 s | repo choice: SJET20 §4.2 "a certain amount of time", no value | repo choice |
| Forager TTL; forwarding-entry lifetime | 10 s; 30 s | repo choice: IJARCCE13 p. 2964 names both, no values | repo choice |
| Event cache size at the source; paths kept per sink | 64 events; 4 paths | repo choice: no open source gives them (golden rule 7: bounded) | repo choice; a sink also opens at most this many paths per scout generation |
| Scout timeout, after which a new forward scout may replace an unanswered one | 1 s | repo choice: no open source gives one | repo choice |
| Table caps: scout cache, forwarding table | 256; 256 | repo choice: no open source gives them (golden rule 7: bounded) | repo choice; oldest evicted first |

## Anchor candidate

| Trend | Scenario | Metric | Comparators | Source |
|---|---|---|---|---|
| BeeSensor has the least energy consumption among the bee algorithms surveyed | not stated in [RF15] | energy consumption | the surveyed bee algorithms | [RF15] Table 2 |
| In a static converge-cast scenario, BeeSensor delivers 90.9 % of events at 18.97 J total energy against EEABR's 92.4 % at 16.66 J and SC's 68.7 % at 14.95 J | RMASE, random topologies of 9–100 nodes, 512-bit events at 4/s per source, 100 s, 10 runs | success rate, energy consumption, energy efficiency | EEABR, SC (ant-based), IEEABR | [ZSAC13] Table 1, Table 2 |

**Not chosen yet; the anchor gate stays open.** The open sources disagree on
BeeSensor's energy. [RF15] Table 2 and IJARCCE13 p. 2964 report it as the least
consumer (against AODV and FP-Ant in IJARCCE13). [ZSAC13] Table 2, a
re-implementation in RMASE, has it consuming more than EEABR and SC in
converge-cast, and far more in target tracking (Table 3). The anchor is
[SUF12]'s own trend once the paper is read. Until then, v0's numbers are not
benchmark results, and nothing is released (swarm-routing/.github#881).

## Differences between [SF07] and [SUF12]

None recorded: neither paper has been read (#25).

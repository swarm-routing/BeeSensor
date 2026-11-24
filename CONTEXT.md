# CONTEXT.md — orientation and history

Background an agent or a new contributor needs that does not belong in
`AGENTS.md`: domain notes, current state, glossary, open questions. Read the
section a task needs, not the whole file.

## Current state

**Status: planned; fidelity sheet first.** The repository carries the org
templates, the release ladder ([`docs/roadmap.md`](docs/roadmap.md)), the
release workflow and the start of the fidelity sheet
([`docs/fidelity.md`](docs/fidelity.md)), with no protocol code. Epic #3 was
deferred on 2026-11-22 (decision 23 on swarm-routing/.github#672): the
fidelity sheet may continue, `core/` code waits until the maintainer re-plans
it.

History of the scope: swarm-routing/.github#54 first listed BeeSensor as out
of scope, as part of the ruled-out low-power duty-cycled radio family
(swarm-routing/AntHocNet#858). The maintainer reversed this on 2026-11-22
(item 10 on swarm-routing/.github#672, recorded on swarm-routing/.github#54
and swarm-routing/.github#275): BeeSensor is in scope with a many-to-one WSN
convergecast family in sim-tools' catalogue (swarm-routing/sim-tools#55),
which is not the sub-GHz duty-cycled mesh. M-IAR
(https://doi.org/10.1109/imtc.2008.4547341), once merged with BeeSensor in a
review, is a different work.

## Relation to BeeAdHoc

BeeSensor is by the same group (Farooq) as BeeHive and BeeAdHoc
(swarm-routing/BeeAdHoc) and carries the bee metaphor (scouts, foragers,
a hive at each node) from MANET routing to sensor networks. Where BeeAdHoc
routes any-to-any between mobile nodes, BeeSensor routes many-to-one from
sensors to a sink and weighs the nodes' remaining energy. What it takes from
BeeAdHoc, and what it changes, is read from [SUF12] and [SF07] into
`docs/fidelity.md`; until then this is the lineage only, not a
specification (golden rule 12).

## Glossary

- **Convergecast:** many-to-one traffic from every sensor to one sink.
- **Sink:** the node that collects the sensors' data.
- **[SUF12], [SF07]:** the two BeeSensor papers (`docs/references.bib`):
  the 2012 journal version, which this repository implements, and the 2007
  workshop paper.

## Open questions

- The papers' texts: both are closed-access (OpenAlex, 2026-11-24). Reading
  them is a maintainer step (#25); every fidelity row waits on it.
- Whether a swarm-core energy port release carries what BeeSensor scores
  (swarm-routing/swarm-core#10).

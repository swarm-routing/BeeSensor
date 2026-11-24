# BeeSensor

<!-- org-sync:begin badges -->
| Group | Badges |
| --- | --- |
| Build | [![Built on: ns3-dev](https://img.shields.io/badge/built%20on-ns3--dev%20image-blue?logo=docker)](https://github.com/orgs/swarm-routing/packages/container/package/ns3-dev) |
| Quality | [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_BeeSensor&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=swarm-routing_BeeSensor) [![Bugs](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_BeeSensor&metric=bugs)](https://sonarcloud.io/summary/new_code?id=swarm-routing_BeeSensor) [![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_BeeSensor&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=swarm-routing_BeeSensor) [![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_BeeSensor&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=swarm-routing_BeeSensor) [![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/swarm-routing/BeeSensor?utm_source=oss&utm_medium=github&utm_campaign=swarm-routing%2FBeeSensor&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)](https://coderabbit.ai) |
| Release | [![Release](https://img.shields.io/github/v/release/swarm-routing/BeeSensor?sort=semver&cacheSeconds=1800)](https://github.com/swarm-routing/BeeSensor/releases) [![Cite](https://img.shields.io/badge/cite-CITATION.cff-blue)](CITATION.cff) [![Licence: GPL-2.0-only, core/ MIT OR GPL-2.0-only](https://img.shields.io/badge/licence-GPL--2.0--only%2C%20core%2F%20MIT%20OR%20GPL--2.0--only-blue)](LICENSING.md) |
| Trust | [![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/swarm-routing/BeeSensor/badge)](https://scorecard.dev/viewer/?uri=github.com/swarm-routing/BeeSensor) [![OpenSSF Best Practices](https://www.bestpractices.dev/projects/17066/badge)](https://www.bestpractices.dev/projects/17066) |
| Stack | [![C++23](https://img.shields.io/badge/C%2B%2B-23-blue?logo=cplusplus)](AGENTS.md) [![Runs on ns-3](https://img.shields.io/badge/runs%20on-ns--3-informational)](https://www.nsnam.org) |
| Community & docs | [![Colony Arena: BeeSensor](https://img.shields.io/badge/Colony%20Arena-project%20page-orange)](https://swarm-routing.github.io/projects/BeeSensor/) [![Open RCAPA](https://img.shields.io/github/issues/swarm-routing/BeeSensor/rcapa?label=open%20RCAPA&color=blue)](https://github.com/swarm-routing/BeeSensor/blob/main/docs/rcapa/README.md) [![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow)](https://www.conventionalcommits.org) |
<!-- org-sync:end badges -->

BeeSensor (Saleem, Ullah & Farooq, [Information Sciences 200, 2012](https://doi.org/10.1016/j.ins.2012.02.024); first published at [EvoWorkshops 2007](https://doi.org/10.1007/978-3-540-71805-5_9)), the honeybee-inspired, energy-aware routing protocol for wireless sensor networks, as a standalone swarm routing protocol: many-to-one convergecast to a sink, scored through swarm-core's energy port (`IEnergy`). It descends from [BeeAdHoc](https://github.com/swarm-routing/BeeAdHoc), the same group's protocol for mobile ad hoc networks.

**Status: planned.** No protocol code yet: the paper-fidelity sheet comes first ([`docs/fidelity.md`](docs/fidelity.md)), then a simulator-free C++ `core/` and an ns-3 module. The release ladder is [`docs/roadmap.md`](docs/roadmap.md); the plan is #3 and swarm-routing/.github#672.

<!-- org-sync:begin develop -->
## Develop

New here? [docs/development.md](docs/development.md) is the developer guide:
the tools to install, how to get the code and its dependencies, how to
build, test, run and lint it, the Docker images, what CI checks and how to
read a red run. How changes are proposed, reviewed and released is the org's
[CONTRIBUTING.md](https://github.com/swarm-routing/.github/blob/main/CONTRIBUTING.md).
Whether a regression here gets an RCAPA (root cause analysis with corrective
and preventive actions), and how: [docs/rcapa/README.md](docs/rcapa/README.md).
<!-- org-sync:end develop -->

<!-- org-sync:begin citing -->
## Releases & citing

Versions follow [SemVer](https://semver.org). Each release is listed under [Releases](https://github.com/swarm-routing/BeeSensor/releases), built by the org's release workflow and archived on [Zenodo](https://zenodo.org) with its own DOI.

To cite this repository, use the “Cite this repository” button (from [CITATION.cff](CITATION.cff)). It has no DOI yet: the concept DOI appears here and in the badge above once Zenodo archives its first release.

Please also cite the work it builds on:

- M. Saleem, I. Ullah, M. Farooq, *BeeSensor: An energy-efficient and scalable routing protocol for wireless sensor networks*, Information Sciences 200, pp. 38–56, 2012, [doi:10.1016/j.ins.2012.02.024](https://doi.org/10.1016/j.ins.2012.02.024). The version this repository implements: the full protocol and its WSN evaluation, the primary source of docs/fidelity.md.

What this implementation reproduces from its sources, and every deliberate deviation, is in [docs/fidelity.md](docs/fidelity.md).

Using it in your work? Contributions are welcome, and citing it helps just as much.
<!-- org-sync:end citing -->

## Epics

<!-- epics:begin -->
<!-- Generated by swarm-routing/.github tools/epics/epics.py from the issues labelled `epic`; edit the issues, then re-run it. -->

What is planned and in progress here: the issues labelled [`epic`](https://github.com/swarm-routing/BeeSensor/issues?q=label%3Aepic). An epic's open sub-issues carry a `priority:*` and a `model:*` label, so a contributor or an AI agent can pick one up ([how issues work](https://github.com/swarm-routing/.github/blob/main/CONTRIBUTING.md#issues)). The whole org's plan: the [roadmap](https://github.com/swarm-routing/.github/blob/main/ROADMAP.md#epics).

| Epic | Status | Sub-issues closed |
|------|--------|-------------------|
| [#3](https://github.com/swarm-routing/BeeSensor/issues/3) Bootstrap BeeSensor protocol on swarm-core (ConvergecastScorer, fidelity, WSN battery-drain benchmarks) | Planned | 0 / 1 |

<!-- epics:end -->

## Licence

GPL-2.0-only, with `core/` MIT OR GPL-2.0-only: see [LICENSING.md](LICENSING.md).

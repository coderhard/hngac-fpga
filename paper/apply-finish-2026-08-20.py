"""Finish the compression pass: checklist rows 15-43, equation \label/\eqref
migration, and the defects introduced while applying rows 1-14."""
import sys

SRC = '/home/abuhassan/projects/hngac-fpga/paper/main-author-rev-2026-08-19.tex'
DST = '/tmp/revbuild/final.tex'

R = []
def rep(tag, old, new):
    R.append((tag, old, new))

# ============================================================
# PART 1 - defects inside the author's own rows 1-14.
# Typos, grammar and logic only. No style relitigation.
# ============================================================

rep('T1 typo+caps', r"""State of the art works approach the various problems from different angles including: Per-decision latency, Worst-case boundedness, dimensionality, and hardfware validation.  To demonstrate novelty, we score prior works on these axes""",
    r"""State-of-the-art work approaches these problems from different angles: per-decision latency, worst-case boundedness, dimensionality, and hardware validation. To demonstrate novelty, we score prior work on these axes""")

rep('T2 logic: offline modifies compilation, not decision latency', r"""XEngine \cite{liu2011xengine}
%is the closest software analogue of our compilation step.
was the first to compile XACML into numeric structures, but its offline decision latency still varies with policy and request shape. Reported measurements like \cite{karim2026tsngac} place OPA \cite{opa} and an XACML PDP  \cite{oasisxacml, servos2017abac} three to five orders""",
    r"""XEngine \cite{liu2011xengine}
%is the closest software analogue of our compilation step.
compiles XACML into numeric structures offline, but its decision latency still varies with policy and request shape. Reported measurements like \cite{karim2026tsngac} place OPA \cite{opa} and an XACML PDP \cite{oasisxacml, servos2017abac} three to five orders""")

rep('T3 undefined abbrev + adverb', r"""\textbf{Hardware enforcement.} Three lines of work approach HW authorization. %Huffmire et al.\ compiled a
Formal specified memory-isolation policy was compiled into an FPGA reference monitor in 2006""",
    r"""\textbf{Hardware enforcement.} Three lines of work approach authorization in hardware. %Huffmire et al.\ compiled a
Formally specified memory-isolation policy was compiled into an FPGA reference monitor in 2006""")

rep('T4 mid-sentence capital + garbled object', r"""Although both make enforcement latency a property of a synthesized circuit, Neither evaluates a relational policy model cost or dimension costs.""",
    r"""Although both make enforcement latency a property of a synthesized circuit, neither evaluates a relational policy model nor prices an added policy dimension.""")

rep('T5 run-on: StrideBV contrast landed on Aker', r"""However, StrideBV latency grows linearly with match-key width, constraining its generalizability. %In the closest deterministic prior art , added dimensions are not free in time, unlike ours.
The Aker model \cite{restuccia2021aker} wraps SoC controllers in verified access-control hardware at the bus-transaction level added dimensions are not free in time, unlike ours.
Interestingly, CHERI \cite{watson2015cheri} enforces per-instruction memory capabilities in silicon. Both police a lower layer than a policy model.""",
    r"""However, StrideBV latency grows linearly with match-key width, constraining its generalizability. In the closest deterministic prior art, added dimensions are not free in time, unlike ours. The Aker model \cite{restuccia2021aker} wraps SoC controllers in verified access-control hardware at the bus-transaction level, and CHERI \cite{watson2015cheri} enforces per-instruction memory capabilities in silicon. Both police a lower layer than a policy model.""")

rep('T6 tense + logic: Contego uses CPU slack', r"""Contego slots security tasks into scheduling slack \cite{hasan2017contego}. SoK \cite{hasan2024sok} addressed scheduling in robot security controls but misses authorization completely. We leave that paradigm rather than optimize within it, because a fabric decision consumes no latency slack.""",
    r"""Contego slots security tasks into scheduling slack \cite{hasan2017contego}. A 2024 SoK \cite{hasan2024sok} catalogs scheduling in robot security controls but misses authorization completely. We leave that paradigm rather than optimize within it, because a fabric decision consumes no CPU slack.""")

rep('T7 typo "my"/"by" + reversed characterization of Pinisetty', r"""Runtime enforcers were synthesized in \cite{pinisetty2017runtime} from timed automata for reactive CPS serving as the nearest formal-synthesis neighbor. Instead of just addressing hardware (HW) authorization, it models safety properties impacted my hardware latency.""",
    r"""Runtime enforcers were synthesized in \cite{pinisetty2017runtime} from timed automata for reactive CPS, the nearest formal-synthesis neighbor to this work. It targets safety properties rather than authorization, and offers no hardware latency model.""")

rep('T8 subject-verb agreement', r"""and \cite{salimi2025abac} condition ABAC decisions on task and capability attributes""",
    r"""and \cite{salimi2025abac} conditions ABAC decisions on task and capability attributes""")

rep('T9 typo "Thus"/"This"', r"""Thus class of threats is adequately addressed with traditional identity triple access control models.""",
    r"""This class of threats is adequately addressed by traditional identity-triple access control models.""")

# ============================================================
# PART 2 - equation \label / \eqref migration.
# ============================================================

rep('EQ label (1)', r"""r_i = (S_i,\, O_i,\, A_i,\, \sigma_i,\, \pi_i)
\end{equation}""",
    r"""r_i = (S_i,\, O_i,\, A_i,\, \sigma_i,\, \pi_i)
\label{eq:rule}
\end{equation}""")

rep('EQ strip stray label from Definition 2', r"""\textbf{Definition 2 (Request\label{eq:request}).}""",
    r"""\textbf{Definition 2 (Request).}""")

rep('EQ label (2)', r"""\wedge\ (\sigma_i \subseteq \sigma_q) \wedge (\pi_i = 0 \vee \pi_i \cap \pi_q \neq \emptyset)
\end{split}
\end{equation}""",
    r"""\wedge\ (\sigma_i \subseteq \sigma_q) \wedge (\pi_i = 0 \vee \pi_i \cap \pi_q \neq \emptyset)
\end{split}
\label{eq:match}
\end{equation}""")

rep('EQ label (3)', r"""\delta(q) = \bigvee_{i=0}^{n-1} \mathit{match}_i(q), \qquad \delta \in \{0,1\}.
\end{equation}""",
    r"""\delta(q) = \bigvee_{i=0}^{n-1} \mathit{match}_i(q), \qquad \delta \in \{0,1\}.
\label{eq:decision}
\end{equation}""")

rep('EQ label (4)', r"""L(n) = 12 + n/2 \ \text{cycles}
\end{equation}""",
    r"""L(n) = 12 + n/2 \ \text{cycles}
\label{eq:latency}
\end{equation}""")

rep('EQ ref -> eqref (match), body', r"""Every term in \ref{eq:request} %(2)
is a bitwise AND""",
    r"""Every term in \eqref{eq:match}
is a bitwise AND""")

rep('EQ ref -> eqref (match), hypothesis', r"""Equation \ref{eq:request} %(2)
makes $\sigma$ and $\pi$""",
    r"""Equation~\eqref{eq:match}
makes $\sigma$ and $\pi$""")

rep('EQ hardcoded (3) in Sec. V opener', r"""The kernel evaluates (3) as a pipelined linear scan.""",
    r"""The kernel evaluates \eqref{eq:decision} as a pipelined linear scan.""")

rep('EQ hardcoded (4) in fig:tail caption', r"""The hardware line is the closed-form bound of (4) at 100~MHz""",
    r"""The hardware line is the closed-form bound of \eqref{eq:latency} at 100~MHz""")

# ============================================================
# PART 3 - checklist rows 15-43.
# ============================================================

rep('15 IV limit behavior', r"""The limit behavior of (3) matters for the reader budgeting a control loop. As $n$ grows toward the 512-rule capacity, hardware cost grows only through scan length, and Section~\ref{sec:arch} shows that growth is exactly half a cycle per rule regardless of how many dimensions each rule carries. In software the same growth compounds with dimensionality: our measured per-rule slope rises from 0.645 cycles in three dimensions to 1.220 cycles in five. Thus the two curves diverge precisely where policy gets rich.""",
    r"""Scaling in $n$ separates the two implementations. As $n$ grows toward the 512-rule capacity, hardware cost grows only through scan length, and Section~\ref{sec:arch} shows that growth is half a cycle per rule regardless of how many dimensions a rule carries. In software the growth compounds with dimensionality: our measured per-rule slope rises from 0.645 cycles in three dimensions to 1.220 in five. The two curves diverge as policy gets richer.""")

rep('16 V memory interface', r"""The policy resides behind a BRAM-style port and the request and rule count arrive over AXI-Lite registers. Synthesis maps the evaluation entirely to LUTs and flip-flops. The kernel consumes zero BRAM and zero DSP blocks, so the policy port is the only memory interface.""",
    r"""The policy sits behind a BRAM-style port and the request and rule count arrive over AXI-Lite registers. Synthesis maps the evaluation entirely to LUTs and flip-flops, and the kernel consumes zero BRAM and zero DSP blocks.""")

rep('17 V figure paragraph', r"""\new{Figure~\ref{fig:systembd} shows the integrated system as built for the board measurement of Section~\ref{subsec:onsilicon}. The kernel sits in the programmable logic as \texttt{hngac\_authorize\_0}. Policy memory is a dual-port block RAM reached through two AXI BRAM controllers, which lets the processing system rewrite policy on one port while the scan reads the other, keeping update traffic off the decision path. The request and rule count arrive over the AXI interconnect from the Zynq processing system. The AXI Timer beside the kernel is the instrument, not part of the primitive: it latches the decision interval in hardware, which is what makes the board cycle counts in Table~\ref{tab:board} measurements of the fabric rather than of the software that called it. The constant 25-cycle offset those counts carry over co-simulation is the round trip through this interconnect.}""",
    r"""\new{Figure~\ref{fig:systembd} shows the system as built for the board measurement of Section~\ref{subsec:onsilicon}. Policy memory is a dual-port block RAM, so the processing system can rewrite policy on one port while the scan reads the other, keeping update traffic off the decision path. The AXI Timer is instrumentation, not part of the primitive. It latches the decision interval in hardware, which is what makes the board counts in Table~\ref{tab:board} measurements of the fabric rather than of its caller. The constant 25-cycle offset those counts carry is the round trip through the interconnect.}""")

rep('18 V synthesis parameter', r"""At 100~MHz, $L(500) = 262$ cycles is 2.62~$\mu$s, and the 512-rule capacity bounds the primitive at 2.68~$\mu$s. A designer needing a tighter bound shrinks $n$ or raises the clock, and either way the bound is known before deployment. That is what we mean by the worst case becoming a synthesis parameter.""",
    r"""At 100~MHz, $L(500) = 262$ cycles is 2.62~$\mu$s, and the 512-rule capacity bounds the primitive at 2.68~$\mu$s. A designer needing a tighter bound shrinks $n$ or raises the clock. Either way the bound is known before deployment, which is what makes the worst case a synthesis parameter.""")

rep('19 V early exit', r"""We note one design consequence plainly. Forgoing early exit costs the average case: a software scan that exits on first match would beat (4) whenever the match lands early. We spend that average case to buy a worst case with zero variance, because the control loop budgets for the worst case and only the worst case.""",
    r"""Forgoing early exit costs the average case. A software scan that exits on first match beats \eqref{eq:latency} whenever the match lands early. We spend the average case to buy a worst case with zero variance, because a control loop budgets against the worst case.""")

rep('20 VI-A three kernels', r"""\new{Three kernels were synthesized and co-simulated: a three-dimensional kernel checking the identity triple, a four-dimensional kernel adding runtime state, and a five-dimensional kernel adding command provenance. The three differ only in the conjunction terms of the rule check; policy layout, pipeline structure, interface and testbench are common.}""",
    r"""\new{Three kernels were synthesized and co-simulated: three-dimensional checking the identity triple, four-dimensional adding runtime state, and five-dimensional adding command provenance. They differ only in the conjunction terms of the rule check. Policy layout, pipeline structure, interface and testbench are common.}""")

rep('21 VI-B corpus', r"""The corpus generator emits, for each policy rule, one request that satisfies all five dimensions, plus a state-failing and a provenance-failing variant where applicable, yielding eleven requests for the four-rule policy and 1{,}334 at 500 rules. The corpus therefore exercises permit paths, state denials and provenance denials at every policy size, and it is the corpus under which the correctness columns of Section~\ref{sec:results} are computed.""",
    r"""For each policy rule the generator emits one request that satisfies all five dimensions, plus a state-failing and a provenance-failing variant where applicable. That yields eleven requests at four rules and 1{,}334 at 500, or 2{,}307 across all six sizes. The corpus exercises permits, state denials and provenance denials at every policy size, and the correctness columns of Section~\ref{sec:results} are computed on it. \new{RTL results use this corpus, sized to co-simulation runtime. The software comparison of Section~\ref{sec:results} uses a 200{,}000-request corpus generated by the same rules, since a software path costs nothing to run at that scale.}""")

rep('22 VI-C honesty rules', r"""We label every number by how it was obtained, and we hold the paper to the labels. Hardware cycle counts are measured and deterministic. No confidence interval applies because variance is zero by construction. Software per-decision cycle counts are derived: perf supplies a measured effective clock of 4.96~GHz, and cycles are mean nanoseconds multiplied by that clock. They are not per-decision counter reads. Proportions from the adversarial run carry exact denominators and one-sided 95\% Wilson score bounds. One baseline, RBAC with a modeled external state lookup, is a synthetic busy-wait. We exclude it from every empirical claim and use the measured SQLite-backed variant instead. The timing-window measurement produced zero slips and is reported only as a null result. \new{Board cycle counts are hardware-latched by an AXI Timer and are measurements; board round-trip latency includes the Linux userspace stack on the processing system and is reported as an observation of that stack, never as a property of the kernel.}""",
    r"""We label every number by how it was obtained. Hardware cycle counts are measured and deterministic, and no confidence interval applies because variance is zero by construction. Software per-decision cycle counts are derived, not counter reads: perf supplies a measured effective clock of 4.96~GHz, and cycles are mean nanoseconds times that clock. Proportions from the adversarial run carry exact denominators and one-sided 95\% Wilson bounds. One baseline, RBAC with a modeled external state lookup, is a synthetic busy-wait, and we exclude it from every empirical claim in favor of the measured SQLite variant. The timing-window measurement produced zero slips and is reported only as a null result. \new{Board cycle counts are hardware-latched by an AXI Timer and are measurements. Board round-trip latency includes the Linux userspace stack on the processing system and is reported as an observation of that stack, never as a property of the kernel.}""")

rep('23 VII-A min mean max', r"""Minimum, mean and maximum are equal at every point\new{, and all three co-simulations pass}. The closed form (4) fits all \new{eighteen} points exactly.""",
    r"""Minimum, mean and maximum are equal at every point\new{, all three co-simulations pass,} and the closed form \eqref{eq:latency} fits all \new{eighteen} points exactly.""")

rep('24 VII-A iteration latency', r"""\new{One row of Table~\ref{tab:resources} is not identical, and we draw attention to it rather than let a reader find it. Iteration latency is two cycles in three dimensions and three in four and five. Pipeline depth grows by one stage when the state dimension arrives and then stops when provenance is added. Because the initiation interval is one, that depth is amortized across the scan and never reaches the per-decision cycle count, which is why Table~\ref{tab:cycles} is flat while pipeline depth is not. The mechanism matters: dimensionality is absorbed into the width of a pipeline stage, not into the length of the pipeline, until a stage runs out of timing slack.}""",
    r"""\new{One row of Table~\ref{tab:resources} is not identical. Iteration latency is two cycles in three dimensions and three in four and five, so pipeline depth grows by one stage when state arrives and then stops when provenance is added. Because the initiation interval is one, that depth is amortized across the scan and never reaches the per-decision cycle count. That is why Table~\ref{tab:cycles} is flat while pipeline depth is not. Dimensionality is absorbed into the width of a pipeline stage, not the length of the pipeline, until a stage runs out of timing slack.}""")

rep('25 VII-A area', r"""\new{Dimensionality} is not free in area, and we do not claim it is. Table~\ref{tab:resources} \new{prices it. The fourth dimension costs 812 LUTs (+21.6\%) over the identity triple and the fifth costs a further 524 (+11.4\%), so carrying both costs 1{,}336 LUTs (+35.5\%) and 331 flip-flops (+14.1\%) against three dimensions. All three kernels consume zero BRAM and zero DSP. The five-dimensional kernel occupies 9\% of a low-cost part against the three-dimensional kernel's 7\%. Free in time, nearly free in area, and we mean the second clause literally: the price of two extra dimensions is roughly two percentage points of a \$100 part.} An earlier four-dimensional build also completed Vivado place and route with +2.170~ns worst negative slack and zero failing endpoints of 1{,}987, so the co-simulated clock is not optimistic for this fabric.""",
    r"""\new{Dimensionality} is not free in area. Table~\ref{tab:resources} \new{prices it. The fourth dimension costs 812 LUTs (+21.6\%) over the identity triple and the fifth a further 524 (+11.4\%), so carrying both costs 1{,}336 LUTs (+35.5\%) and 331 flip-flops (+14.1\%). All three kernels use zero BRAM and zero DSP. The five-dimensional kernel occupies 9\% of the part against the three-dimensional kernel's 7\%, so two added dimensions cost about two percentage points of a \$100 device.} An earlier four-dimensional build completed Vivado place and route with +2.170~ns worst negative slack and zero failing endpoints of 1{,}987, so the co-simulated clock is not optimistic for this fabric.""")

rep('26 VII-B opener', r"""\new{Table~\ref{tab:cycles} shows three kernels spending the same cycles. Table~\ref{tab:rtlauth} shows them answering different questions with those cycles. We ran the identical 2{,}307-request corpus through all three co-simulations, so these are decisions taken by silicon-bound RTL, not by a software model of it.}""",
    r"""\new{Table~\ref{tab:cycles} shows three kernels spending the same cycles. Table~\ref{tab:rtlauth} shows them answering different questions. We ran the identical 2{,}307-request corpus through all three co-simulations, so these are RTL decisions, not software model decisions.}""")

rep('27 VII-B 3D permits all', r"""\new{The three-dimensional kernel permits the entire corpus, all 2{,}307 requests, at every policy size. This is not a defect in the kernel. Every request in the corpus carries a valid identity triple, and the variants differ only in the state and provenance fields, so a kernel that cannot represent $\sigma$ or $\pi$ has no basis on which to deny any of them. It is correct with respect to the question it can ask. The question is simply not the one Layla's interlock asks.}""",
    r"""\new{The three-dimensional kernel permits the entire corpus, all 2{,}307 requests, at every policy size. This is not a defect. Every request carries a valid identity triple and the variants differ only in the state and provenance fields, so a kernel that cannot represent $\sigma$ or $\pi$ has no basis to deny any of them. It answers its own question correctly. That question is not the one Layla's interlock asks.}""")

rep('28 VII-B gaps (FIX: read across the total row)', r"""\new{The four-dimensional kernel denies the state-failing variants and admits the provenance-failing ones, permitting 1{,}587. The five-dimensional kernel permits 864, exactly the satisfying request for each rule at each policy size. Reading down a row, 1{,}443 requests separate three dimensions from five, and 723 of those separate four from five. Each of those two gaps is an attack class: unsafe-state operation in the first, command provenance abuse in the second, anchored respectively in CVE-2022-33323 and CVE-2021-38425.}""",
    r"""\new{The four-dimensional kernel denies the state-failing variants and admits the provenance-failing ones, permitting 1{,}587. The five-dimensional kernel permits 864, exactly one satisfying request per rule at each policy size. Across the total row, 1{,}443 requests separate three dimensions from five, and 723 of those separate four from five. Each gap is an attack class: unsafe-state operation (CVE-2022-33323) and command provenance abuse (CVE-2021-38425).}""")

rep('29 VII-B columns move independently', r"""\new{The point of putting Table~\ref{tab:cycles} and Table~\ref{tab:rtlauth} on facing evidence is that the columns move independently. The cycle columns are identical and the permit columns are not. In hardware, the security question a kernel can ask is decoupled from what asking it costs in time.}""",
    r"""\new{The two tables move independently. The cycle columns are identical and the permit columns are not. In hardware, the security question a kernel can ask is decoupled from what asking it costs in time.}""")

rep('30 VII-C software slopes (+ the abstract 19%)', r"""The identical kernel logic, compiled for the i7 and measured under perf, tells the opposite story. Per-rule marginal cost rises from 0.645 cycles in three dimensions to 1.024 in four and 1.220 in five. That is, in software every rule pays for every dimension on every decision, and the cost compounds with policy size: at 500 rules the five-dimensional software path spends 685 derived cycles per decision against the hardware kernel's 262 measured cycles. Figure~\ref{fig:keyfinding} plots both families\new{, and the contrast is the paper in one image}. The software curves fan out as dimensions are added. \new{The three hardware curves are one line.}""",
    r"""The same kernel logic compiled for the i7 and measured under perf tells the opposite story. Per-rule marginal cost rises from 0.645 cycles in three dimensions to 1.024 in four and 1.220 in five, \new{a 19\% increase for the fifth dimension alone}. In software every rule pays for every dimension on every decision, and the cost compounds with policy size. At 500 rules the five-dimensional software path spends 685 derived cycles per decision against the hardware kernel's 262 measured cycles. Figure~\ref{fig:keyfinding} plots both families. The software curves fan out as dimensions are added\new{, and the three hardware curves are one line}.""")

rep('31 VII-D baselines (FIX: "April corpus")', r"""Table~\ref{tab:models} compares six software models on the same corpus. The RBAC hash map resolves in roughly 100 derived cycles at every policy size, and the flat cost is genuine. However, it admits 200{,}000 of 200{,}000 requests in the April corpus, including all 100{,}000 that violate state, and the DAG-NGAC and three-dimensional paths admit the same. A model that cannot see the state or provenance dimensions cannot deny on them. The only software models that decide correctly are the five-dimensional path and a flattened direct-lookup table that trades a 19.7$\times$ memory footprint for its speed. However, an RBAC deployment could encode state into its role set, a maintenance operator role distinct from a normal operator role. Sixteen state bits and three source types multiply that role count combinatorially, and the honest alternative, RBAC consulting an external state store, cost 20.6 times the four-dimensional path when we measured it against an in-process SQLite store. It might be tempting to read Table~\ref{tab:models} as an argument that cheap static authorization suffices. The correctness column is the answer: the fast rows are fast because they answer a smaller question than the one Layla's interlock asks.""",
    r"""Table~\ref{tab:models} compares six software models on the 200{,}000-request corpus. The RBAC hash map resolves in roughly 100 derived cycles at every policy size, and that flat cost is genuine. It also admits 200{,}000 of 200{,}000 requests, including all 100{,}000 that violate state, and the DAG-NGAC and three-dimensional paths admit the same. A model that cannot see state or provenance cannot deny on them. The only software models that decide correctly are the five-dimensional path and a flattened direct-lookup table that buys its speed with 19.7$\times$ the memory. An RBAC deployment could encode state into its roles, a maintenance operator distinct from a normal operator, but sixteen state bits and three source types multiply that role count combinatorially. The alternative, RBAC consulting an external state store, cost 20.6 times the four-dimensional path against an in-process SQLite store. Table~\ref{tab:models} is not an argument that cheap static authorization suffices. The fast rows are fast because they answer a smaller question than the one Layla's interlock asks.""")

rep('31b VII-D table: put the over-authorization count in the table', r"""\caption{Software models, derived cycles per decision at 500 rules, and corpus correctness. ``Over'' = admits state- or provenance-violating requests.}
\label{tab:models}
\centering
\begin{tabular}{lrl}
\toprule
Model & Cycles (500) & Decides correctly? \\
\midrule
H-NGAC 3D & 390 & No, over-authorizes \\
H-NGAC 4D & 582 & No, over-authorizes \\
H-NGAC 5D & 685 & \textbf{Yes} \\
RBAC hash map & 100 & No, over-authorizes \\
DAG-NGAC traversal & 1{,}032 & No, over-authorizes \\
Flattened 5D lookup & 984 & \textbf{Yes}, 19.7$\times$ memory \\
\bottomrule
\end{tabular}""",
    r"""\caption{Software models, derived cycles per decision at 500 rules, and corpus correctness. \new{``Admits invalid'' counts requests permitted despite violating state or provenance, out of 200{,}000.}}
\label{tab:models}
\centering
\footnotesize
\setlength{\tabcolsep}{4pt}
\begin{tabular}{lrrl}
\toprule
Model & Cycles (500) & \new{Admits invalid} & Correct? \\
\midrule
H-NGAC 3D & 390 & \new{100{,}000} & No \\
H-NGAC 4D & 582 & \new{50{,}000} & No \\
H-NGAC 5D & 685 & \new{0} & \textbf{Yes} \\
RBAC hash map & 100 & \new{100{,}000} & No \\
DAG-NGAC traversal & 1{,}032 & \new{100{,}000} & No \\
Flattened 5D lookup & 984 & \new{0} & \textbf{Yes}\rlap{, 19.7$\times$ mem.} \\
\bottomrule
\end{tabular}""")

rep('32 VII-E adversarial', r"""We ran the provenance-abuse scenario live. A software gatekeeper node executing the same five-dimensional decision function fronted an actuator topic while a legitimate node and a compromised node both published with valid credentials for the same authorized subject, the compromised node at five times the legitimate rate. Over a 30-second session the policy blocked 18{,}878 of 18{,}878 injection attempts, a block rate of 100\% with a one-sided 95\% Wilson lower bound of 99.99\%, and passed 17{,}059 of 17{,}059 legitimate commands, a false positive rate of 0\% with a 95\% upper bound of 0.02\%. This demo and the board run carry two separate claims. The demo establishes what the five-dimensional policy semantics block under sustained adversarial load. The silicon run establishes that the synthesized kernel computes identical decisions on all 2{,}307 corpus requests. Neither claim substitutes for the other. Under three- or four-dimensional policy every one of those injections would have been admitted, because the attacker's identity triple and state context were valid. Only the provenance dimension separates the two traffic streams.""",
    r"""We ran the provenance-abuse scenario live. A software gatekeeper node running the same five-dimensional decision function fronted an actuator topic while a legitimate node and a compromised node both published with valid credentials for the same subject, the compromised node at five times the legitimate rate. Over a 30-second session the policy blocked 18{,}878 of 18{,}878 injection attempts, a 100\% block rate with a one-sided 95\% Wilson lower bound of 99.99\%, and passed 17{,}059 of 17{,}059 legitimate commands, a 0\% false positive rate with a 95\% upper bound of 0.02\%. This run and the silicon run carry separate claims. The first establishes what the five-dimensional semantics block under sustained adversarial load, the second that the synthesized kernel reproduces those decisions on all 2{,}307 corpus requests. Under three- or four-dimensional policy every injection would have been admitted, because the attacker's identity triple and state context were valid. Only provenance separates the two streams.""")

rep('33 VII-E timing window', r"""The same runs measured the timing-window property. Under both idle and saturated CPU load, zero authorization callbacks of 8{,}733 exceeded the 50~$\mu$s lower bound of DDS localhost propagation. We report this strictly as a null result. The architectural argument stands on (4): a synthesis-time bound below the propagation floor closes the window by construction, which no measured software mean can do.""",
    r"""The same runs measured the timing window. Under idle and saturated CPU load, zero of 8{,}733 authorization callbacks exceeded the 50~$\mu$s lower bound of DDS localhost propagation. We report this as a null result. The argument stands on \eqref{eq:latency}: a synthesis-time bound below the propagation floor closes the window by construction, which no software mean can do.""")

rep('34 VII-F opener', r"""\new{Co-simulation is cycle accurate, but it is still simulation. We placed the five-dimensional kernel on a PYNQ-Z1 at 100~MHz behind an AXI Timer and read the decision interval out of a hardware-latched counter. Table~\ref{tab:board} reports the result against the co-simulated counts.}""",
    r"""\new{Co-simulation is cycle accurate but still simulation. We placed the five-dimensional kernel on a PYNQ-Z1 at 100~MHz behind an AXI Timer and read the decision interval from a hardware-latched counter. Table~\ref{tab:board} reports it against the co-simulated counts.}""")

rep('35 VII-F board tracks', r"""\new{The board tracks co-simulation with a constant 25-cycle offset at all six policy sizes, and minimum, mean and maximum are equal on silicon exactly as they are in simulation. The offset is the AXI-Lite handshake that moves the request into the kernel and the decision back out, and being constant it enters the closed form of (4) as an additive term rather than a source of variance. The determinism argued from pipeline structure in Section~\ref{sec:arch} therefore survives contact with the fabric, the interface, and the part.}""",
    r"""\new{The board tracks co-simulation with a constant 25-cycle offset at all six policy sizes, and minimum, mean and maximum are equal on silicon as they are in simulation. The offset is the AXI-Lite handshake that moves the request in and the decision out. Being constant, it enters \eqref{eq:latency} as an additive term rather than a source of variance. The determinism argued from pipeline structure in Section~\ref{sec:arch} holds on the real part.}""")

rep('36 VII-F round trip', r"""\new{The same board run measured round-trip latency from the processing system, and that number behaves differently. Mean round trip rises from 2.09~$\mu$s at four rules to 4.77~$\mu$s at 500, but the maximum reaches 5.92~ms at 500 rules, more than three orders of magnitude above its own mean. We report this because it is the honest counterpart to Table~\ref{tab:board} and because it makes the paper's argument on the paper's own hardware. The fabric is invariant and the counter proves it. The millisecond tail is contributed entirely by the Linux userspace stack on the board's Cortex-A9 between the timer read and the application, which is the identical mechanism we attribute the i7's tail to in Section~\ref{subsec:comparison}. A deterministic primitive does not confer determinism on a non-deterministic caller. It bounds the part of the path a designer can bound, and Table~\ref{tab:board} is the part we claim.}""",
    r"""\new{The same run measured round-trip latency from the processing system, and that number behaves differently. Mean round trip rises from 2.09~$\mu$s at four rules to 4.77~$\mu$s at 500, but the maximum reaches 5.92~ms at 500 rules, three orders of magnitude above its own mean. The tail comes entirely from the Linux userspace stack on the board's Cortex-A9 between the timer read and the application, the same mechanism behind the i7's tail in Section~\ref{subsec:comparison}. A deterministic primitive does not make a non-deterministic caller deterministic. It bounds the part of the path a designer can bound, and Table~\ref{tab:board} is that part.}""")

rep('37 VII-G heading', r"""\subsection{The comparison we do not make, and the one we do}""",
    r"""\subsection{Mean versus worst case}""")

rep('38 VII-G concession', r"""It might be tempting to infer from Table~\ref{tab:cycles} that the fabric outpaces the CPU. That inference would be incorrect, and we pre-empt it. At 500 rules""",
    r"""The fabric does not outpace the CPU, and we say so directly. At 500 rules""")

rep('39 VII-G worst case', r"""The defensible comparison is the worst case, and there the ordering reverses: the software path's worst observed decision at 200 rules was 355.4~$\mu$s against the fabric's invariant 1.12~$\mu$s, and its worst at four rules was 298.8~$\mu$s against 0.14~$\mu$s. Further, the software figure is an observation, not a bound. The 157~$\mu$s outlier in the DCAS baseline arrived in a run whose mean was 1.14~$\mu$s \cite{karim2026dcas}. An operating system does not guarantee its own tail, and a control loop budgets for the tail. Figure~\ref{fig:tail} plots both concessions on one wall-clock axis. The software mean runs below the hardware bound at every policy size. The software worst case runs above it at every size, by more than three orders of magnitude at four rules, and it does not track policy size, because the tail belongs to the scheduler and not to the policy.""",
    r"""The defensible comparison is the worst case, and there the ordering reverses. The software path's worst observed decision at 200 rules was 355.4~$\mu$s against the fabric's invariant 1.12~$\mu$s, and its worst at four rules was 298.8~$\mu$s against 0.14~$\mu$s. Further, the software figure is an observation, not a bound: the 157~$\mu$s outlier in the DCAS baseline arrived in a run whose mean was 1.14~$\mu$s \cite{karim2026dcas}. An operating system does not guarantee its own tail, and a control loop budgets for the tail. Figure~\ref{fig:tail} plots both. The software mean runs below the hardware bound at every policy size and the worst case runs above it at every size, by more than three orders of magnitude at four rules. The worst case does not track policy size, because the tail belongs to the scheduler and not to the policy.""")

rep('40 VII-G general engines', r"""For completeness we place general-purpose policy engines on the same axis. In an earlier evaluation, Open Policy Agent averaged 271.491~$\mu$s per decision with 13 deadline misses \cite{karim2026tsngac}, and an XACML OpenPDP deployment averaged roughly 50~ms \cite{karim2026dcas, oasisxacml}. These engines evaluate far richer policy than (2) expresses, and we do not fault their generality. We observe only that they occupy a latency regime three to five orders of magnitude above the closed form of (4), which is the regime that keeps authorization out of control paths.""",
    r"""General-purpose policy engines sit on the same axis. In an earlier evaluation, Open Policy Agent averaged 271.491~$\mu$s per decision with 13 deadline misses \cite{karim2026tsngac}, and an XACML OpenPDP deployment averaged roughly 50~ms \cite{karim2026dcas, oasisxacml}. These engines evaluate far richer policy than \eqref{eq:match} expresses. They also occupy a latency regime three to five orders of magnitude above \eqref{eq:latency}, the regime that keeps authorization out of control paths.""")

rep('41 VIII limitations', r"""Our claims are bounded by several factors, and we state them plainly. \new{First, the parity result covers three, four and five dimensions on one part at one clock, and the three kernels differ only in the conjunction terms of the rule check. We have not shown that an arbitrary sixth dimension is free, only that these two were, and Section~\ref{sec:arch} names the mechanism that would end the pattern.} \new{Second,} the software baseline is a 4.96~GHz laptop CPU while the fabric runs at 100~MHz; an embedded-class CPU baseline on the board's own Cortex-A9 would make the wall-clock comparison fairer to the fabric, and we did not run it. \new{Our board data bounds the fabric and the interface but not a software decision path on the same silicon.} \new{Third,} software latency distributions were measured under WSL2, whose scheduler jitter contaminates maxima, which is why we cite worst observed values and never call them WCETs. \new{Fourth,} the primitive is capacity-bounded at 512 rules and 256 policy nodes, and policy update happens off the decision path. \new{Fifth,} one baseline, the modeled external-state lookup, is synthetic, and we excluded it from every empirical claim after finding it added nothing the measured SQLite path does not show. \new{Sixth,} the kernel compiles only the association subset of INCITS 565. Prohibitions, obligations and administrative relations are not implemented, so no claim here extends beyond rules that grant. \new{Seventh,} the zero-cycle result is demonstrated at one operating point, 100~MHz on a Zynq-7020, where \new{all three} kernels close timing with identical +0.33~ns slack. At an aggressive clock target \new{an} added dimension could force an additional pipeline stage, which is exactly the boundary the hypothesis of Section~\ref{sec:model} predicts\new{, and the two-to-three step in iteration latency between three and four dimensions shows the mechanism already operating below the threshold where it would cost cycles}. Characterizing that frequency ceiling is future work.""",
    r"""\new{First, the parity result covers three, four and five dimensions on one part at one clock. We have not shown that an arbitrary sixth dimension is free, only that these two were, and Section~\ref{sec:arch} names the mechanism that would end the pattern.} \new{Second,} the software baseline is a 4.96~GHz laptop CPU while the fabric runs at 100~MHz. A baseline on the board's own Cortex-A9 would make the wall-clock comparison fairer, and we did not run it. \new{The board data bounds the fabric and the interface, not a software decision path on the same silicon.} \new{Third,} software latency distributions were measured under WSL2, whose scheduler jitter contaminates maxima, which is why we cite worst observed values and never call them WCETs. \new{Fourth,} the primitive is capacity-bounded at 512 rules and 256 policy nodes, and policy update happens off the decision path. \new{Fifth,} the modeled external-state lookup is synthetic and excluded from every empirical claim, since the measured SQLite path shows the same thing. \new{Sixth,} the kernel compiles only the association subset of INCITS 565, so no claim extends beyond rules that grant. \new{Seventh,} the zero-cycle result holds at one operating point, 100~MHz on a Zynq-7020, where \new{all three} kernels close timing with +0.33~ns slack. At an aggressive clock target \new{an} added dimension could force another pipeline stage, the boundary the hypothesis of Section~\ref{sec:model} predicts\new{. The two-to-three step in iteration latency already shows that mechanism operating below the threshold where it costs cycles}. Characterizing the frequency ceiling is future work.""")

rep('42 IX future work', r"""Four questions structure our next steps. \new{1.~At what clock frequency does the next dimension stop being free, and does that ceiling move with policy width or only with stage depth? 2.~What does a decision path in software on the board's own Cortex-A9 cost, so the wall-clock comparison runs at one class of silicon?} 3.~Does the closed form of (4) survive a dynamic policy update path, or does update traffic reintroduce jitter? 4.~How does the primitive compose with time-scoped delegation, which TS-NGAC develops in software \cite{karim2026tsngac}? Further, we call on the community to report authorization performance as cycle-level worst cases rather than means, because a mean is precisely the statistic a control loop cannot budget against.""",
    r"""Four questions follow. \new{1.~At what clock does the next dimension stop being free, and does that ceiling move with policy width or only with stage depth? 2.~What does a software decision path on the board's own Cortex-A9 cost, so the wall-clock comparison runs at one class of silicon?} 3.~Does \eqref{eq:latency} survive a dynamic policy update path, or does update traffic reintroduce jitter? 4.~How does the primitive compose with time-scoped delegation, which TS-NGAC develops in software \cite{karim2026tsngac}? Further, we call on the community to report authorization performance as cycle-level worst cases rather than means, because a mean is the statistic a control loop cannot budget against.""")

rep('43 IX conclusion', r"""In conclusion, we extended the H-NGAC bitmask primitive with system-state and command-provenance dimensions and synthesized \new{three-, }four- and five-dimensional kernels to a Zynq-7020. The added \new{dimensions} cost zero clock cycles and \new{1{,}336} LUTs, latency is closed-form at $12 + n/2$ cycles with zero jitter by construction \new{and is confirmed on silicon by a hardware-latched counter, the same corpus that the five-dimensional kernel denies 1{,}443 times is permitted in full by the three-dimensional kernel at identical cycle cost}, and the five-dimensional policy blocked 18{,}878 of 18{,}878 credentialed injection attempts in ROS~2 with zero false positives in 17{,}059 legitimate commands. Robotics platform engineers and functional-safety engineers can treat the authorization worst case as a synthesis parameter, place the decision inside the control loop budget, and close the two attack classes that identity-based authorization admits. Layla's interlock becomes a policy bit the fabric checks in bounded time, not a procedure she hopes the operator follows.""",
    r"""We extended the H-NGAC bitmask primitive with system-state and command-provenance dimensions and synthesized \new{three-, }four- and five-dimensional kernels to a Zynq-7020. The added \new{dimensions} cost zero clock cycles and \new{1{,}336} LUTs. Latency is closed-form at $12 + n/2$ cycles with zero jitter by construction\new{, confirmed on silicon by a hardware-latched counter. At identical cycle cost, the three-dimensional kernel permits in full the same corpus the five-dimensional kernel denies 1{,}443 times}, and the five-dimensional policy blocked 18{,}878 of 18{,}878 credentialed injection attempts in ROS~2 with zero false positives in 17{,}059 legitimate commands. Robotics and functional-safety engineers can treat the authorization worst case as a synthesis parameter, place the decision inside the control loop budget, and close the two attack classes identity-based authorization admits. Layla's interlock becomes a policy bit the fabric checks in bounded time, not a procedure she hopes the operator follows.""")

src = open(SRC).read()
fail = 0
for tag, old, new in R:
    c = src.count(old)
    if c != 1:
        print(f"!! [{tag}] matched {c}x: {old[:80]!r}")
        fail += 1
        continue
    src = src.replace(old, new)
open(DST, 'w').write(src)
print(f"applied {len(R)-fail}/{len(R)}")
sys.exit(1 if fail else 0)

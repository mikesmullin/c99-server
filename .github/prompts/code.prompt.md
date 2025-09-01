---
name: Code Review
description: A prompt for reviewing and improving code snippets.
author: Mike Smullin
type: agent
---

## Objective
Review and suggest improvements for the provided code snippets.

## Context
The attached/referenced files in this VSCode workspace.

## Instructions
- Carefully read the code provided.
- Identify any areas for improvement, including but not limited to:
   - Code readability
   - Performance optimization
   - Adherence to best practices
- Think of constructive feedback and suggestions for improvement.

## Examples & Constraints
- IMPORTANT! Adhere to my coding style preferences; read [docs/my-code-style.md](../../docs/my-code-style.md) 

## Output Format
- Present your plan for how you might amend the attached code.
- Ask me to approve before proceeding to implement the changes on my behalf.
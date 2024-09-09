# Slash

# Project Title

Project delevoped using Unreal Engine version 5.2 during the Unreal Engine 5 C++ The Ultimate Game Developer Course created and taugh by [Stephen Ulibarri](https://www.udemy.com/user/stephen-ulibarri-3/).

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

## Table Of Content

- [About The Project](#about-the-project)
- [Skills Learned](#skills-learned)
  - [Animation Montage and Animation Blueprint](#animation-montage-and-animation-blueprint)
  - [Motion Warping](#motion-warping)
  - [Enemy Behavior](enemy-behavior)
- []()

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

## 🕹️ About The Project

In this project, a third-person character, called Slash, was developed to walk around an open world map, have combat skills and defeat enemies inside a dungeon full of enemies and a final boss.

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

## 🎯 Skills Learned

While following the course and building this project, I was able to develop skills in Unreal Engine, game development, refresh some C++ concepts as well as get used to how UE make use of abstractions to simplify the code in general.
I have also encountered many challenges while implementing extra features such as more enemy animations, lock on target mechanic, work with NavMesh Bounds Volume, and so on. I'll list below some skills that really impressed me while I was learning.

### Animation Montage and Animation Blueprint

#### 🧩 Animation Montage:

In this project many animations were used and the majority of them were combined in many animation montages (AM) so it could be controlled through C++ code as well as Blueprint script.

I've learned how to add and split sections so I could decide when to play an animation based on conditions in the code (like the current state of the character);  
Add events using notifies in order to control box collision for the weapon during attack montages, to identify the end of each animation sequence. Use notifies to also add sounds, and motion warping windows which can direct the root motion towards a set target.

![alt text](./Screenshots/rm-echo-attack.png)

#### 🧩 Animation Blueprint (ABP):

It allows the usage of data and logic to determine which animation to play. To make use of the data, we need to grab a reference of the character that has the skeletal mesh using the ABP. In order to do that, the returning pawn has to be casted and set to a pointer of the character class type we're using, called SlashCharacter:

![alt text](./Screenshots/anim-instance-cpp.png)

With its reference we can now get all data we need to use in the ABP and update the output pose based on logic. State Machines hold the logic, and having many State Machines is better than having a single one with many states and transition rules, that can easily become confusing. Another way to keep things organized is to use Linked Anim Graph node that lets us use animations blueprint within another. In this project there's a main ABP, ABP_Echo, that uses two other animations blueprints, ABP_Echo_MainStates, and ABP_Echo_IK.

![alt text](./Screenshots/abp-echo.png)

![alt text](./Screenshots/abp-echo-main-states.png)

![alt text](./Screenshots/abp-echo-ik.png)

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

### Motion Warping

As mentioned above, Motion Warping is added to an animation sequence in animation montage through notifies. To use this feature, we need to enable the Motion Warping plugin, use animations that has root motion enabled, meaning the skeletal mesh must have a root bone to make it all work, and add a motion warping component to the character's blueprint. In this project, we use two motion warpings: one that warps the root motion translation component and another that warps the rotation component.

For this project, motion warping is used when a character has a valid combat target, making the attack animation warp to that target location. In the event graph of the character using a motion warping component, every frame we'll check whether there's a combat target. If so we'll call a custom function we created in C++, GetTransLationWarpTarget and hook the return value to the Add Or Update Warp Target from Location node from the motion warping component. That node also needs the warp target name which in this case corresponds to the motion warping named TranslationTarget. After that we'll call another function we've implemented GetRotationWarpTarget and use the return value to the next Add Or Update Warp Target from Location node which now uses the motion warping named RotationTarget that will make the character face the target.

![alt text](./Screenshots/motion-warping-bp.png)

![alt text](./Screenshots/motion-warping-cpp.png)

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

### Enemy Behavior

GIF/VIDEO

The enemy class was the longest to develop, meaning there was a lot to learn and implement. The AI behavior was implemented with methods using states to check what was the current state in order to play a certain animation or to choose another one; we also make use of the navigation .

I've spent a good amount of time trying to debug an issue when we implemented a second enemy, a Raptor. The bug really annoyed me because even though this new enemy was a child of the Enemy C++ class, the first enemy created, BP_Paladin, would not have the bug.
It happens that the Raptor stopped patrolling after some time doing it. So I've used debug spheres, placing in the Raptor's location and the next patrol target to figure out what was wrong. I checked it stopped choosing the next patrol target out of the patrol targets array.

After long hours trying the issue, the solution was in finding the best Capsule Componenet shape. I've noticed that tweaking both Capsule Radius and Capsule Half Height, the enemy would finally choose the next patrol target to move to. That also happened when I later added another enemy, BP_Vampire which was even bigger than the Raptor, but that time I knew all I had to do was adjust the Capsule Componenet shape.

[![Raptor patrolling bug](https://img.youtube.com/vi/bsPqVMfWcHY&ab_channel=EvelineGeorgia/0.jpg)](https://www.youtube.com/watch?v=bsPqVMfWcHY&ab_channel=EvelineGeorgia)

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

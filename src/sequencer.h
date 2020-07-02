#ifndef MOD_SEQUENCER
#define MOD_SEQUENCER

/*
 

  (define mainscript 
    (create-states
      (state closed
        (attributes (color "blue"))     ; this should set the color attribute to blue, but then set it back when not in closed state
        (track "opening door" 
          (display "opening door")
          (play-animation open-door-animation)     
          (trigger "door-opened")
        )
        (track "play shake head"
          (wait-trigger "door-opened") 
          (display "player shaking head")
          (play-animation player-shake-head)
          (go-state open)
        )
        (track "music"
          (play-sound-clip "hello")
          (play-sound-clip "world")   ; this can get preempted, if go-state open happens before. 
        )
      )
      (state open

      )
    )
  )
*/

#endif